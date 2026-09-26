//////////////////////////////////////////////////////////////////////////////////////
// fpsprite.h - Fang point sprite module.
//
// Author: Steve Ranck     
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
// 05/31/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FPSPRITE_H_
#define _FPSPRITE_H_ 1

#include "fang.h"
#include "fmath.h"
#include "ftex.h"
#include "fcolor.h"
#include "fviewport.h"
#include "fxfm.h"


#define FPSPRITE_BYTE_ALIGNMENT			16	// FPSprite_t's must be aligned to this boundary

enum {
	FPSPRITE_FLAG_DONT_DRAW				= 0x00000001,	// SET=Don't draw this group
	FPSPRITE_FLAG_NO_FOG				= 0x00000002,	// SET=Don't apply fog
	FPSPRITE_FLAG_ENABLE_DEPTH_WRITES	= 0x00000004,	// SET=Enable depth buffer writes, FALSE=disable depth buffer writes (compares are always enabled)
	FPSPRITE_FLAG_SORT					= 0x00000008,	// SET=Have world system sort this group in relation to other groups and objects
	FPSPRITE_FLAG_SIZE_IN_PIXELS		= 0x00000010,	// SET=PS size is in screen pixels, FALSE=PS size is in model space
	FPSPRITE_FLAG_NOCLIP				= 0x00000020,	// SET=The particles in this group are guaranteed to never cross a frustum plane
	FPSPRITE_FLAG_UNIFORM_ZBASEDSCALE	= 0x00000040,	// SET=The particles in this group can be uniformly scaled based on the bounding sphere's z-distance from camera, rather than individually (faster on some platforms)

	FPSPRITE_FLAG_NONE					= 0
};


typedef enum {
	FPSPRITE_BLEND_MODULATE,		// No texture: C = LERP( Av, Cfb, Cv ),  With texture: C = LERP( Av*At, Cfb, Cv*Ct )
	FPSPRITE_BLEND_ADD,				// No texture: C = Cfb + Cv,  With texture: C = Cfb + Cv*Ct

	FPSPRITE_BLEND_COUNT
} FPSpriteBlend_e;


typedef struct {					// IMPORTANT: Do NOT change this structure:
	CFVec3 Point_MS;				// The point sprite's center point in model space
	f32 fDim_MS;					// The point sprite's horizontal and vertical dimension in model units (this may be capped by the hardware)
	CFColorRGBA ColorRGBA;			// The point sprite's color (RGB) and opaqueness (A)
} FPSprite_t;


FCLASS_ALIGN_PREFIX class CFPSpriteGroup {
public:
	FINLINE CFPSpriteGroup() { m_nMaxCount=0; m_pBase=NULL; m_nRenderCount=0; m_nRenderStartIndex=0; m_fEmulationDistance = 0.0f; }

	void ComputeBoundingSphere();	// fills in m_BoundSphere_MS from all active sprites
	void ComputeMinMaxBox( CFVec3 &rMin, CFVec3 &rMax ) const;// computes a min/max box from all of the active sprites

	FViewportPlanesMask_t Render( BOOL bPerformFrustumTest=TRUE, FViewportPlanesMask_t nCrossesPlanesMask=FVIEWPORT_PLANESMASK_ALL, CFMtx43A *pMirrorMtx = NULL );

	FINLINE FPSprite_t *GetFirstPS( void ) const;
	FINLINE FPSprite_t *GetLastPS( void ) const;
	FINLINE u32 GetFirstPSIndex( void ) const;
	FINLINE u32 GetLastPSIndex( void ) const;
	FINLINE u32 GetRenderCount_Start( void ) const;
	FINLINE u32 GetRenderCount_End( void ) const;
	FINLINE void GetRenderCounts( u32 *pnStartRenderCount, u32 *pnEndRenderCount ) const;
	FINLINE void IncRenderStartIndex( void );
	FINLINE void DecRenderStartIndex( void );
	FINLINE void IncRenderCount( void );
	FINLINE void DecRenderCount( void );

private:
	#if !FANG_PLATFORM_GC
		FViewportPlanesMask_t _RenderEmulatedGroup( BOOL bPerformFrustumTest, FViewportPlanesMask_t nCrossesPlanesMask );
	#endif

public:
	CFXfm m_Xfm;					// This group's transformation matrix
	CFSphere m_BoundSphere_MS;		// This group's bounding sphere in model space
	f32 m_fCullDist;				// Don't draw this group if its distance to the camera is greater than this value (MAX_FLOAT = don't distance cull)
	u32 m_nPSFlags;					// Flags (see FPSPRITE_FLAG_* for info)
	FPSpriteBlend_e m_nBlend;		// Blend operation
	CFTexInst m_TexInst;			// Texture to use (set m_TexInst to NULL to use solid colored point sprites in this group)

	u32 m_nMaxCount;				// Number of point sprites m_pBase can hold
	FPSprite_t *m_pBase;			// Pointer to point sprite buffer base (must be aligned on a FPSPRITE_BYTE_ALIGNMENT byte boundary)

	u32 m_nRenderCount;				// Number of point sprites to render (if m_nRenderCount>m_nMaxCount, rendering will wrap around to buffer start)
	u32 m_nRenderStartIndex;		// Index into m_pPSArray of first point sprite to render
	f32 m_fEmulationDistance;		// The distance from the origin of the group at which to use the emulated render path over the HW render path.

	FCLASS_STACKMEM_ALIGN( CFPSpriteGroup );
} FCLASS_ALIGN_SUFFIX;


FINLINE FPSprite_t *CFPSpriteGroup::GetFirstPS( void ) const {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	return m_pBase + m_nRenderStartIndex;
}

FINLINE FPSprite_t *CFPSpriteGroup::GetLastPS( void ) const {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( (m_nRenderStartIndex + m_nRenderCount) <= m_nMaxCount ) {
		return m_pBase + (m_nRenderStartIndex + m_nRenderCount - 1);
	} else {
		return m_pBase + (m_nRenderStartIndex + m_nRenderCount - m_nMaxCount - 1);
	}
}

FINLINE u32 CFPSpriteGroup::GetFirstPSIndex( void ) const {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	return m_nRenderStartIndex;
}

FINLINE u32 CFPSpriteGroup::GetLastPSIndex( void ) const {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( (m_nRenderStartIndex + m_nRenderCount) <= m_nMaxCount ) {
		return m_nRenderStartIndex + m_nRenderCount - 1;
	} else {
		return m_nRenderStartIndex + m_nRenderCount - m_nMaxCount - 1;
	}
}

FINLINE u32 CFPSpriteGroup::GetRenderCount_Start( void ) const {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( (m_nRenderStartIndex + m_nRenderCount) <= m_nMaxCount ) {
		return m_nRenderCount;
	} else {
		return m_nMaxCount - m_nRenderStartIndex;
	}
}

FINLINE u32 CFPSpriteGroup::GetRenderCount_End( void ) const {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( (m_nRenderStartIndex + m_nRenderCount) <= m_nMaxCount ) {
		return 0;
	} else {
		return m_nRenderStartIndex + m_nRenderCount - m_nMaxCount;
	}
}

FINLINE void CFPSpriteGroup::GetRenderCounts( u32 *pnStartRenderCount, u32 *pnEndRenderCount ) const {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( (m_nRenderStartIndex + m_nRenderCount) <= m_nMaxCount ) {
		*pnStartRenderCount = m_nRenderCount;
		*pnEndRenderCount = 0;
	} else {
		*pnStartRenderCount = m_nMaxCount - m_nRenderStartIndex;
		*pnEndRenderCount = m_nRenderCount - *pnStartRenderCount;
	}
}

FINLINE void CFPSpriteGroup::IncRenderStartIndex( void ) {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( ++m_nRenderStartIndex == m_nMaxCount ) {
		m_nRenderStartIndex = 0;
	}
}

FINLINE void CFPSpriteGroup::DecRenderStartIndex( void ) {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( m_nRenderStartIndex == 0 ) {
		m_nRenderStartIndex = m_nMaxCount;
	}
	m_nRenderStartIndex--;
}

FINLINE void CFPSpriteGroup::IncRenderCount( void ) {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( m_nRenderCount < m_nMaxCount ) {
		m_nRenderCount++;
	}
}

FINLINE void CFPSpriteGroup::DecRenderCount( void ) {
	FASSERT( m_nRenderStartIndex < m_nMaxCount );
	FASSERT( m_nRenderCount <= m_nMaxCount );

	if( m_nRenderCount ) {
		m_nRenderCount--;
	}
}



extern BOOL FPSprite_bEmulated;			// TRUE=Emulated (not directly supported by hardware)
extern f32 FPSprite_fLargestSize_SS;	// The largest size (horizontal and vertical) in screen space that the hardware supports (0.0f=no limit)


extern BOOL fpsprite_ModuleStartup( void );
extern void fpsprite_ModuleShutdown( void );


extern void fpsprite_Renderer_Open( void ); 
extern void fpsprite_Renderer_Close( void ); 
extern u32  fpsprite_Renderer_GetStateSize( void ); 
extern void fpsprite_Renderer_GetState( void *pDestState ); 
extern void fpsprite_Renderer_SetState( const void *pState ); 
extern void fpsprite_Renderer_SetDefaultState( void ); 



#endif

