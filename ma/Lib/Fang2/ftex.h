//////////////////////////////////////////////////////////////////////////////////////
// ftex.h - Fang texture module.
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

#ifndef _FTEX_H_
#define _FTEX_H_ 1

#include "fang.h"
#include "fdata.h"

#include "fclib.h"


// if you are looking for FTEX_RESNAME, see fres.h - Mike

//#define FTEX_TEXNAME_LEN					15

#define FTEX_SOURCE_IMAGE_BYTE_ALIGNMENT	16

#define FTEX_DEFAULT_RENDER_TARGET_NAME		"RenderTarget"

extern char FTex_szNewReflectMap[16];	// Reflection map that should be used instead of FTex_szOldReflectMap
extern char FTex_szOldReflectMap[16];	// The reflection map that should be swapped to the FTex_szNewReflectMap

#if FANG_PLATFORM_PS2
	// PlayStation2:

	typedef enum 
	{
		FTEX_FMT_A8B8G8R8,					// 32-bit, with 8-bit alpha
//ARG		FTEX_FMT_X8B8G8R8,					// 32-bit
		FTEX_FMT_A4R4G4B4,					//ARG - 16-bit, with 4-bit alpha
		FTEX_FMT_A1B5G5R5,					// 16-bit, with 1-bit alpha
		FTEX_FMT_P8,						// 8-bit paletted
		FTEX_FMT_P4,						// 4-bit paletted

		FTEX_FMT_COUNT
	} FTexFmt_e;

	typedef enum 
	{
		FTEX_PALFMT_NONE,

		FTEX_PALFMT_A8B8G8R8,				// 32-bit, with 8-bit alpha
		FTEX_PALFMT_A1B5G5R5,				// 16-bit, with 1-bit alpha

		FTEX_PALFMT_COUNT
	} FTexPalFmt_e;

#elif FANG_PLATFORM_DX
	// DX8:

	typedef enum 
	{
		FTEX_FMT_A8R8G8B8,					// 32-bit, with 8-bit alpha
		FTEX_FMT_X8R8G8B8,					// 32-bit
		FTEX_FMT_A4R4G4B4,					// 16-bit, with 4-bit alpha
		FTEX_FMT_A1R5G5B5,					// 16-bit, with 1-bit alpha
		FTEX_FMT_X1R5G5B5,					// 16-bit
		FTEX_FMT_R5G6B5,					// 16-bit
		FTEX_FMT_DXT1,						// DXT1 compressed
		FTEX_FMT_DXT3,						// DXT3 compressed
		FTEX_FMT_DXT5,						// DXT5 compressed
		FTEX_FMT_X8L8V8U8,					// 32bpp EMBM texture format, can also be used for 3 vector bumpmapping (like dot3)

		FTEX_FMT_COUNT
	} FTexFmt_e;

	typedef enum 
	{
		FTEX_PALFMT_NONE,

		FTEX_PALFMT_COUNT
	} FTexPalFmt_e;

#elif FANG_PLATFORM_GC
	// GameCube:

	typedef enum 
	{
		FTEX_FMT_R8G8B8A8,					// 32-bit, 8-bit alpha
		FTEX_FMT_R8G8B8X8,					// 32-bit, opaque (alpha forced to 255)
		FTEX_FMT_R4G4B4A3,					// 16-bit, 3-bit alpha (Most significant bit must be 0, then 3 bits of Alpha, 4 R, 4 G, 4 B)
		FTEX_FMT_R5G5B5X1,					// 15-bit, opaque  (ONLY FOR INTERNAL FANG USAGE - Most significant bit must be 1)
		FTEX_FMT_R5G6B5,					// 16-bit, opaque
		FTEX_FMT_S3TC,						// 4-bit compressed RGB, opaque
		FTEX_FMT_S3TCA1,					// 4-bit compressed RGB, 1-bit alpha
		FTEX_FMT_S3TCx2,					// pair of S3TC textures, one 4-bit RGB representing color, one 4-bit rep. Alpha in green channel
		FTEX_FMT_I8,						// 8-bit intensity map
		FTEX_FMT_IA8,						// 8-bit intensity + 8-bit alpha map (used for bumpmapping) - 16bpp
		FTEX_FMT_IA4,						// 4-bit intensity + 4-bit alpha map (used for bumpmapping) - 8bpp
		FTEX_FMT_I4,

		FTEX_FMT_COUNT
	} FTexFmt_e;

	typedef enum 
	{
		FTEX_PALFMT_NONE,

		FTEX_PALFMT_COUNT
	} FTexPalFmt_e;
	
#else
	#error <Fang Error: Undefined texture types for platform.>

#endif


typedef enum 
{
	FTEX_RENDERTARGET_FMT_C16,				// 16-bit color
	FTEX_RENDERTARGET_FMT_C16_D16,			// 16-bit color, 16-bit depth
	FTEX_RENDERTARGET_FMT_C16_D15_S1,		// 16-bit color, 15-bit depth, 1-bit stencil
	FTEX_RENDERTARGET_FMT_C16_D32,			// 16-bit color, 32-bit depth
	FTEX_RENDERTARGET_FMT_C16_D24_S8,		// 16-bit color, 24-bit depth, 8-bit stencil

	FTEX_RENDERTARGET_FMT_C15_A1,			// 15-bit color, 1-bit alpha
	FTEX_RENDERTARGET_FMT_C15_A1_D16,		// 15-bit color, 1-bit alpha, 16-bit depth
	FTEX_RENDERTARGET_FMT_C15_A1_D15_S1,	// 15-bit color, 1-bit alpha, 15-bit depth, 1-bit stencil
	FTEX_RENDERTARGET_FMT_C15_A1_D32,		// 15-bit color, 1-bit alpha, 32-bit depth
	FTEX_RENDERTARGET_FMT_C15_A1_D24_S8,	// 15-bit color, 1-bit alpha, 24-bit depth, 8-bit stencil

	FTEX_RENDERTARGET_FMT_C24_A8,			// 24-bit color, 8-bit alpha
	FTEX_RENDERTARGET_FMT_C24_A8_D16,		// 24-bit color, 8-bit alpha, 16-bit depth
	FTEX_RENDERTARGET_FMT_C24_A8_D15_S1,	// 24-bit color, 8-bit alpha, 15-bit depth, 1-bit stencil
	FTEX_RENDERTARGET_FMT_C24_A8_D32,		// 24-bit color, 8-bit alpha, 32-bit depth
	FTEX_RENDERTARGET_FMT_C24_A8_D24_S8,	// 24-bit color, 8-bit alpha, 24-bit depth, 8-bit stencil
	
	#if FANG_PLATFORM_GC
	FTEX_RENDERTARGET_FMT_I8_D24,
	FTEX_RENDERTARGET_FMT_I4_D24,
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
	FTEX_RENDERTARGET_FMT_I8_D24,
	FTEX_RENDERTARGET_FMT_I4_D24,
//ARG - <<<<<
	#endif

	FTEX_RENDERTARGET_FMT_COUNT
} FTexRenderTargetFmt_e;


enum 
{
	FTEX_FLAG_DYNAMIC		= 0x01,		// Allow application to modify texture content dynamically
	FTEX_FLAG_VIDMEM		= 0x02,		// Put texture directly into video memory
	FTEX_FLAG_RENDERTARGET	= 0x04,		// Texture is a render target
	FTEX_FLAG_CUBEMAP		= 0x08,
	FTEX_FLAG_LOAD_IN_PLACE	= 0x40,		// Load in place texture - pre-swizzled or compressed
	FTEX_FLAG_STREAMING		= 0x80,		// Texture is in streaming remote memory
	
	FTEX_FLAG_NONE			= 0x00
};



typedef struct _FTexPalData_s FTexPalData_t;
//typedef struct _FTexDef_s FTexDef_t;
typedef struct _FTexData_s FTexData_t;

typedef void *FTexFrame_t;


//
//
//
typedef struct 
{
	FTexPalFmt_e nPalFmt;				// Color and alpha format for palette entries
	void *pData;						// Pointer to palette data
} FTexPalette_t;


//
//
//
struct FTexInfo_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FTexInfo_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	char szName[FDATA_TEXNAME_LEN+1];	// ASCIIZ name of texture
	void *pUserData;					// Pointer to user-defined data

	u8 nTexFmt;							// Texel format (See FTexFmt_e)
	u8 nPalFmt;							// Palette format (See FTexPalFmt_e)

	u8 nFlags;							// See FTEX_FLAG_* for info
	u8 nLodCount;						// Number of LODs. 1=not mipmapped. >1 for mipmapped images

	u8 nRenderTargetStencilBitCount;	// For render targets, this is the number of bits in the stencil buffer
	u8 nRenderTargetDepthBitCount;		// For render targets, this is the number of bits in the depth buffer

	u8 anReserved[2];					// Unused

	u16 nTexelsAcross;					// Number of texels across of largest LOD image (always a power of 2)
	u16 nTexelsDown;					// Number of texels down of largest LOD image (always a power of 2)

//ARG - >>>>>
#ifdef _MMI_TARGET_PS2
    s32 nKBias;
    u8 nLBias;
    u8 nMipMapFilter;

	u16	m_nBlockOffset[7];	// NS VRAM blockoffsets of each MipMap texture
	u32 m_nDataSize;		// NS Just used to provide Pasm with processed data size
	BOOL m_nSwizzled;		// NS Was this texture pre-swizzled
#endif
//ARG - <<<<<

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() 
	{ 
		u32 i;
		for( i=0; i < (FDATA_TEXNAME_LEN+1); i++ ) 
		{
			szName[i] = fang_ConvertEndian( szName[i] );
		}
		pUserData = fang_ConvertEndian( pUserData );
		nTexFmt = fang_ConvertEndian( nTexFmt );
		nPalFmt = fang_ConvertEndian( nPalFmt );
		nFlags = fang_ConvertEndian( nFlags );
		nLodCount = fang_ConvertEndian( nLodCount );
		nRenderTargetStencilBitCount = fang_ConvertEndian( nRenderTargetStencilBitCount );
		nRenderTargetDepthBitCount = fang_ConvertEndian( nRenderTargetDepthBitCount );
		for( i=0; i < sizeof( anReserved ); i++ ) 
		{
			anReserved[i] = fang_ConvertEndian( anReserved[i] );
		}
		nTexelsAcross = fang_ConvertEndian( nTexelsAcross );
		nTexelsDown = fang_ConvertEndian( nTexelsDown );
	}
};


//
//
//
struct FTexDef_t 
{
	FTexInfo_t TexInfo;					// Public texture info. See fTexInfo_t for details
	FTexData_t *pTexData;				// Private texture info used internally by Fang
};


typedef enum
{
	FTEX_RVIEW_PERSP=0,
	FTEX_RVIEW_ORTHO2D,
	FTEX_RVIEW_ORTHO3D,
} FTex_RenderView_e;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFTexInst
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CFTexInst 
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  CFTexInst is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////


	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
	public:

		enum 
		{
			FLAG_WRAP_S						= 0x00000001,	// TRUE: Wrap the S coordinate of the texture (FALSE=clamp)
			FLAG_WRAP_T						= 0x00000002,	// TRUE: Wrap the T coordinate of the texture (FALSE=clamp)
			FLAG_WRAP_U						= 0x00000004,	// TRUE: Wrap the U coordinate of the texture (FALSE=clamp)
			FLAG_MAGFILTER_NO_BILINEAR		= 0x00000008,	// TRUE: Disable bilinear interpolation on the mag filter
			FLAG_MINFILTER_NO_BILINEAR		= 0x00000010,	// TRUE: Disable bilinear interpolation on the min filter
			FLAG_MINFILTER_NO_TRILINEAR		= 0x00000020,	// TRUE: Disable trilinear interpolation between mipmap levels
			FLAG_RENDERTARGET				= 0x00000040,	// TRUE: Use as a RenderTarget.
			FLAG_FULLSCREEN					= 0x00000080,   // TRUE: Fullscreen RenderTarget.
			FLAG_POW2						= 0x00000100,	// TRUE: Pow2 RenderTarget.
			FLAG_FLUSH_RENDERTARGET			= 0x00000200,	// TRUE: Force a draw to the same buffer for this frame.
			FLAG_NORELEASE					= 0x00000400,
			FLAG_CUBEMAP					= 0x00000800,
			FLAG_FORCENOLAG					= 0x00001000,	// Force no lag when updating render target and using it, used for shadows.
		
		#if FANG_PLATFORM_GC
			// GameCube only:
			FLAG_STATE_HAS_CHANGED			= 0x80000000,	// TRUE: One or more of WrapS, WrapT, WrapU, MagBi, MinBi, MinTri, m_fMipmapBias, or m_pTexDef has changed
		#endif

			FLAG_NONE						= 0x00000000
		};




	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
	public:

		// Platform independent:
		FINLINE CFTexInst()							{ SetToDefaults(); }

	#if 1//FANG_PLATFORM_DX
		FINLINE void SetToDefaults()				{ m_pTexDef=NULL; m_nFlags=FLAG_WRAP_S|FLAG_WRAP_T; m_fMipmapBias=0.0f; m_nBufferIdx = 0; }
	#else
		FINLINE void SetToDefaults()				{ m_pTexDef=NULL; m_nFlags=FLAG_NONE; m_fMipmapBias=0.0f; }
	#endif

		FINLINE FTexDef_t *GetTexDef() const		{ return m_pTexDef; }
		FINLINE f32 GetMipmapBias() const			{ return m_fMipmapBias; }
		FINLINE u32 GetFlags() const				{ return m_nFlags; }

		// Platform specific versions of these are implemented below:
		FINLINE CFTexInst &operator = ( const CFTexInst &rTexInst );
		FINLINE BOOL operator == ( const CFTexInst &rTexInst ) const;
		FINLINE BOOL operator != ( const CFTexInst &rTexInst ) const;

	#if 1//FANG_PLATFORM_DX
		//RenderTarget API
		//FULLSCREEN
		BOOL BeginRender_FullScreen();
		void EndRender_FullScreen(BOOL bAutoClearRGBA=FALSE, BOOL bAutoClearDepth=FALSE);
		//SIDEBAR
		BOOL BeginRender();
		void EndRender(BOOL bAutoClearRGBA=FALSE, BOOL bAutoClearDepth=FALSE);
		//End RenderTarget API

		FINLINE void SetTexBufferData(FTexData_t *pData0, FTexData_t *pData1);
	#endif
		
	#if FANG_PLATFORM_DX
		FINLINE void SetTexDef( FTexDef_t *pTexDef );
	#endif
		FINLINE void SetMipmapBias( f32 fMipmapBias );
		FINLINE void SetFlags( u32 nNewFlags );
		FINLINE void SetFlag( u32 nFlag );
		FINLINE void ClearFlag( u32 nFlag );
		FINLINE void FlipFlags( u32 nFlipMask );

		FINLINE FTexData_t *GetTexBuffer(int idx) { return (m_pTexBuffer[idx]); }


		static CFTexInst NullTexInst;


	#if FANG_PLATFORM_GC
		// GameCube only:

		void SetTexDef( FTexDef_t *pTexDef );
		FINLINE BOOL HasStateChanged( void ) const;
		FINLINE void SetStateChangedFlag( BOOL bSetFlag );
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
		// PlayStation2 only
		void SetTexDef( FTexDef_t *pTexDef );
//ARG - <<<<<
	#endif

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
	private:

	//	void *m_pPlatformData;				// Platform specific texture instance data
		FTexDef_t *m_pTexDef;				// Pointer to TexDef to use

	#if 1//FANG_PLATFORM_DX
		FTexData_t *m_pTexBuffer[2];		// Double buffer texture data for RenderTargets
		u8 m_nBufferIdx;
	#endif

		u32 m_nFlags;						// See FTEX_INSTFLAG_* for info
		f32 m_fMipmapBias;					// 0.0f=normal, -1=bias by one smaller level, +1=bias by one larger level, etc.


	public:

		void ChangeEndian( void )
		{
			m_pTexDef = (FTexDef_t *)fang_ConvertEndian( m_pTexDef );
			m_nFlags = fang_ConvertEndian( m_nFlags );
			m_fMipmapBias = fang_ConvertEndian( m_fMipmapBias );
		}

		FCLASS_STACKMEM_NOALIGN( CFTexInst );
} FCLASS_NOALIGN_SUFFIX;



#if FANG_PLATFORM_DX
	// Xbox and Windows implementation:

	FINLINE CFTexInst &CFTexInst::operator = ( const CFTexInst &rTexInst ) 
	{
		m_pTexDef = rTexInst.m_pTexDef;
		m_nFlags = rTexInst.m_nFlags;
		m_fMipmapBias = rTexInst.m_fMipmapBias;
		return *this;
	}

	FINLINE BOOL CFTexInst::operator == ( const CFTexInst &rTexInst ) const 
	{
		return (m_pTexDef == rTexInst.m_pTexDef) && (m_nFlags == rTexInst.m_nFlags) && (m_fMipmapBias == rTexInst.m_fMipmapBias);
	}

	FINLINE BOOL CFTexInst::operator != ( const CFTexInst &rTexInst ) const 
	{
		return (m_pTexDef != rTexInst.m_pTexDef) || (m_nFlags != rTexInst.m_nFlags) || (m_fMipmapBias != rTexInst.m_fMipmapBias);
	}

	FINLINE void CFTexInst::SetTexDef( FTexDef_t *pTexDef ) 
	{
		m_pTexDef = pTexDef;
	}

#if 1
	FINLINE void CFTexInst::SetTexBufferData(FTexData_t *pData0, FTexData_t *pData1) 
	{
		m_pTexBuffer[0] = pData0; m_pTexBuffer[1] = pData1; 
	}
#endif

	FINLINE void CFTexInst::SetMipmapBias( f32 fMipmapBias ) 
	{
		m_fMipmapBias = fMipmapBias;
	}

	FINLINE void CFTexInst::SetFlags( u32 nNewFlags ) 
	{
		m_nFlags = nNewFlags;
	}

	FINLINE void CFTexInst::SetFlag( u32 nFlag ) 
	{
		m_nFlags |= nFlag;
	}

	FINLINE void CFTexInst::ClearFlag( u32 nFlag ) 
	{
		m_nFlags &= ~nFlag;
	}

	FINLINE void CFTexInst::FlipFlags( u32 nFlipMask ) 
	{
		m_nFlags ^= nFlipMask;
	}


#elif FANG_PLATFORM_GC
	// GameCube implementation:

	FINLINE CFTexInst &CFTexInst::operator = ( const CFTexInst &rTexInst ) 
	{
		if( *this != rTexInst ) 
		{
			m_pTexDef = rTexInst.m_pTexDef;
			m_nFlags = rTexInst.m_nFlags;
			m_fMipmapBias = rTexInst.m_fMipmapBias;

			SetStateChangedFlag( TRUE );
		}

		return *this;
	}


	FINLINE BOOL CFTexInst::operator == ( const CFTexInst &rTexInst ) const
	{
		return (m_pTexDef == rTexInst.m_pTexDef)
				&& (((m_nFlags ^ rTexInst.m_nFlags) & ~FLAG_STATE_HAS_CHANGED) == 0)
				&& (m_fMipmapBias == rTexInst.m_fMipmapBias);
	}


	FINLINE BOOL CFTexInst::operator != ( const CFTexInst &rTexInst ) const 
	{
		return (m_pTexDef != rTexInst.m_pTexDef)
				|| (((m_nFlags ^ rTexInst.m_nFlags) & ~FLAG_STATE_HAS_CHANGED) != 0)
				|| (m_fMipmapBias != rTexInst.m_fMipmapBias);
	}


	FINLINE BOOL CFTexInst::HasStateChanged( void ) const 
	{
		return (BOOL)( m_nFlags & FLAG_STATE_HAS_CHANGED );
	}


	FINLINE void CFTexInst::SetStateChangedFlag( BOOL bSetFlag ) 
	{
		if( bSetFlag ) 
		{
			FMATH_SETBITMASK( m_nFlags, FLAG_STATE_HAS_CHANGED );
		} 
		else 
		{
			FMATH_CLEARBITMASK( m_nFlags, FLAG_STATE_HAS_CHANGED );
		}
	}
	

	void CFTexInst::SetTexDef( FTexDef_t *pTexDef );

	#if 1
	FINLINE void CFTexInst::SetTexBufferData(FTexData_t *pData0, FTexData_t *pData1) 
	{
		m_pTexBuffer[0] = pData0; m_pTexBuffer[1] = pData1; 
	}
	#endif

	FINLINE void CFTexInst::SetMipmapBias( f32 fMipmapBias ) 
	{
		if( fMipmapBias != m_fMipmapBias ) 
		{
			SetStateChangedFlag( TRUE );
			m_fMipmapBias = fMipmapBias;
		}
	}


	FINLINE void CFTexInst::SetFlags( u32 nNewFlags ) 
	{
//		nNewFlags &= (FLAG_WRAP_S|FLAG_WRAP_T|FLAG_WRAP_U|FLAG_MAGFILTER_NO_BILINEAR|FLAG_MINFILTER_NO_BILINEAR|FLAG_MINFILTER_NO_TRILINEAR);

		if( ((nNewFlags ^ m_nFlags) & ~FLAG_STATE_HAS_CHANGED) ) 
		{
			m_nFlags = nNewFlags;
			SetStateChangedFlag( TRUE );
		}
	}
	
	FINLINE void CFTexInst::SetFlag( u32 nFlag ) 
	{
		m_nFlags |= nFlag;
	}

	FINLINE void CFTexInst::ClearFlag( u32 nFlag ) 
	{
		m_nFlags &= ~nFlag;
	}

	FINLINE void CFTexInst::FlipFlags( u32 nFlipMask ) 
	{
		if( nFlipMask ) 
		{
			m_nFlags ^= nFlipMask;
			SetStateChangedFlag( TRUE );
		}
	}


//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	// PlayStation2 implementation:

	//ARG - same as XB version at the moment
	FINLINE CFTexInst &CFTexInst::operator = ( const CFTexInst &rTexInst ) {
		m_pTexDef = rTexInst.m_pTexDef;
		m_nFlags = rTexInst.m_nFlags;
		m_fMipmapBias = rTexInst.m_fMipmapBias;
		return *this;
	}


	FINLINE BOOL CFTexInst::operator == ( const CFTexInst &rTexInst ) const {
		return (m_pTexDef == rTexInst.m_pTexDef) && (m_nFlags == rTexInst.m_nFlags) && (m_fMipmapBias == rTexInst.m_fMipmapBias);
	}


	FINLINE BOOL CFTexInst::operator != ( const CFTexInst &rTexInst ) const {
		return (m_pTexDef != rTexInst.m_pTexDef) || (m_nFlags != rTexInst.m_nFlags) || (m_fMipmapBias != rTexInst.m_fMipmapBias);
	}


	FINLINE void CFTexInst::SetMipmapBias( f32 fMipmapBias ) {
		m_fMipmapBias = fMipmapBias;
	}


	FINLINE void CFTexInst::SetFlags( u32 nNewFlags ) {
		m_nFlags = nNewFlags;
	}

	FINLINE void CFTexInst::SetFlag( u32 nFlag ) {
		m_nFlags |= nFlag;
	}

	FINLINE void CFTexInst::ClearFlag( u32 nFlag ) {
		m_nFlags &= ~nFlag;
	}

	FINLINE void CFTexInst::FlipFlags( u32 nFlipMask ) {
		m_nFlags ^= nFlipMask;
	}
//ARG - <<<<<

#else
	#error <Need platform-specific implementation of CFTexInst>
#endif


extern u32 FTex_nMaxTextureWidth;		// Maximum texture width allowed (for largest LOD)
extern u32 FTex_nMaxTextureHeight;		// Maximum texture height allowed (for largest LOD)
extern u32 FTex_nMaxAspect;				// Maximum allowed aspect ratio (width/height or height/width)

extern BOOL FTex_bMipmaps;				// TRUE=mipmaps are supported
extern BOOL FTex_bRenderTargets;		// TRUE=render targets are supported
extern BOOL FTex_bDynamicTextures;		// TRUE=dynamic textures are supported
extern BOOL FTex_bDynamicsCanBeLost;	// TRUE=dynamic textures can be lost on this platform (FALSE on PS2 and XB)




//---------------------------------------------------------------------------------------------------------------------
// ftex_ModuleStartup()
// ftex_ModuleShutdown()
//
// Starts up and shuts down this module, respectively.
// Called by the Fang initialization system.
// Returns TRUE if successful, or FALSE if the module could not successfully initialized.
//
// nMaxTextures indicates to the tex system how much memory it should allocate to hold
// texture descriptors and other per-texture information.
//
// nTexMemBytes is the amount of memory the tex module may allocate and use for storing
// texture image data. This memory is allocated and managed by the tex module through
// a frame-based memory allocation scheme. This parameter is ignored for DX8 builds
// because DX8 manages its own texture memory.
extern BOOL ftex_ModuleStartup( void );
extern void ftex_ModuleShutdown( void );



//---------------------------------------------------------------------------------------------------------------------
// ftex_ComputeMipmapLevelCount()
//
// Given the specified texel format and texture dimensions of the highest-level LOD,
// this function returns the maximum number of mipmap levels supported by the platform.
// Note that when computing the LOD dimensions, the dimensions must be greater than 1
// for normal textures and greater than 4 for DXT textures. The dimensions passed in
// must comply with FTex_nMaxTextureWidth, FTex_nMaxTextureHeight, and FTex_nMaxAspect,
// for PlayStation2 and X-Box, but do not need to comply with Windows.
//
// If mipmapping is not supported, this function always returns 1.
extern u32 ftex_ComputeMipmapLevelCount( FTexFmt_e nTexelFormat, u32 nTexelsAcross, u32 nTexelsDown );



extern FTexDef_t *ftex_Find( cchar *pszName );



//---------------------------------------------------------------------------------------------------------------------
// ftex_IsPointerValid()
//
// Performs a quick validity check on the provided pointer. Returns TRUE if it is
// likely to be a valid FTexDef_t pointer, or FALSE otherwise. Usually used within
// the FASSERT() macro to check for validity during debug builds.
//
// If pTexDef is NULL, this function returns bNullPointerIsValid.
extern BOOL ftex_IsPointerValid( const FTexDef_t *pTexDef, BOOL bNullPointerIsValid );



//---------------------------------------------------------------------------------------------------------------------
// ftex_CreateTexture()
// ftex_ModifyTexture()
//
// Use ftex_CreateTexture() to create a texture in the current frame. All data
// in *pTexInfo must have been initialized prior to calling this function. pSrcImage
// points to the actual texture image data, organized appropriately according to
// its texel type. pPalette points to the texture's palette, if any, and may be
// NULL if no palette is required. This function passes pack a texture definition
// pointer, FTexDef_t which the app may use to track the texture and later modify it.
// If NULL is passed back, the texture could not be created.
//
// The caller must insure that the texture dimensions, texel format, and all other
// characteristics of the texture adhere to the limitations of the system. Use
// the various public variables offered through the tex module to determine these
// limitations. If the texture is not compliant, it will not be created and NULL
// is returned.
//
// If the app sets the FTEX_FLAG_DYNAMIC flag in pTexInfo::nFlags, the texture is
// flagged as a dynamic texture. Dynamic textures are unique in that the app may
// modify the texture image data after it has been created. Depending on the
// hardware, dynamic textures might be more expensive to render than normal textures.
//
// The FTEX_FLAG_VIDMEM flag is a hint that tells ftex_CreateTexture() to place
// the image directly into video memory. Once loaded, the texture permanently resides
// in video memory until a ftex_ReleaseFrame() destroys it. This leaves less room
// for normal textures and may reduce texture performance if overly used.
//
// The FTEX_STAT_RENDERTARGET flag must not be set. See ftex_CreateRenderTarget()
// for information on render targets.
//
// Use ftex_ModifyTexture() to modify a texture image or its palette. Only the image
// and palette data may be modified. No other characteristic of a texture can be
// modified once it has been created. The texture must have been created as a dynamic
// texture, or this function simply returns without doing anything. This function
// cannot be used on render targets. To not modify the image data, pass NULL for
// pSrcImage. To not modify the palette data, pass NULL for pPalette. If the texture
// is a non-paletted texture, pPalette must be NULL.
//
// In both of these functions, the data that the parameters point to does not need
// to persist beyond the call. Fang copies the data into internal areas.
//
// Note that pSrcImage must always be aligned to FTEX_SOURCE_IMAGE_BYTE_ALIGNMENT.
extern FTexDef_t *ftex_CreateTexture( const FTexInfo_t *pTexInfo, const void *pSrcImage, const FTexPalette_t *pPalette, FResHandle_t hRes=FRES_NULLHANDLE, FResHandle_t *phRes=NULL );
extern BOOL ftex_ModifyTexture( FTexDef_t *pTexDef, const void *pSrcImage, const FTexPalette_t *pPalette );

#if FANG_PLATFORM_WIN
extern u32 ftex_ComputeConversion( FTexFmt_e nSrcTexFmt, u32 nSrcLodCount, u32 nSrcWidth, u32 nSrcHeight,
								   FTexFmt_e *pnDstTexFmt, u32 *pnDstLodCount, u32 *pnDstWidth, u32 *pnDstHeight );
extern BOOL ftex_Convert( FTexFmt_e nSrcTexFmt, u32 nSrcLodCount, u32 nSrcWidth, u32 nSrcHeight, const void *pSrcImage,
						  FTexFmt_e nDstTexFmt, u32 nDstLodCount, u32 nDstWidth, u32 nDstHeight, void *pDstImage );
#endif

extern void ftex_PreLoad( FTexDef_t *pTexDef );


//------------------------------------------------------
// USING RENDER TARGETS:
// CFTexInst *_pFScrTest=NULL, *_pSmallTest=NULL;
//
//********Create a Callback function for rendering***************
//
//void _TestFullScreenCallback(void)
//{
//	fviewport_Clear(FVIEWPORT_CLEARFLAG_COLOR | FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0);
//	fvis_Draw( NULL, FALSE );
//}
//
//void _TestSmallCallback(void)
//{
//	fviewport_Clear(FVIEWPORT_CLEARFLAG_COLOR | FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0);
//	fvis_Draw( NULL, FALSE );
//}
//
//*********************CreateRenderTarget() or CreateRenderTarget_FullScreen(), maximum non-fullscreen size is 128x128************
//*********************AddRenderTarget() - pass in texture (CFTexInst), Callback, RenderWorld flag, max update frequency (frame rate), autoclear********
//void _SetupTarget()
//{
//	if (!_pFScrTest && !_pSmallTest)
//	{
//		_pFScrTest = ftex_CreateRenderTarget_FullScreen(FTEX_RENDERTARGET_FMT_C16_D16, "FullScreenTest");
//		ftex_AddRenderTarget(_pFScrTest, _TestFullScreenCallback, TRUE, 0, FALSE, TRUE);
//
//		_pSmallTest = ftex_CreateRenderTarget(128, 128, FTEX_RENDERTARGET_FMT_C16_D16, "SmallTest");
//		ftex_AddRenderTarget(_pSmallTest, _TestSmallCallback, TRUE, 5, FALSE, TRUE);
//	}
//}
//Thats it, whenever its time to update your rendertarget, your callback will be called.


//---------------------------------------------------------------------------------------------------------------------
// ftex_CreateRenderTarget()
//
// Use this function to create a render target. Once created, the render
// target behaves just like a normal texture with the exception that it can
// be rendered into.
//
// Render targets are expensive compared to normal textures. Once created, they
// reside permanently in video memory until an ftex_ReleaseFrame() destroys it.
// Render targets leave less room for normal textures and may reduce texture
// performance if overly used.
//
// nTexelsAcross and nTexelsDown indicate the dimensions of the render target.
// The dimensions are hints to the function. The function will attempt to create
// the render target at or close to the specified size, but might change the
// dimensions. This function also attempts to maintain the aspect ratio, but
// will change it if necessary.
//
// nDesiredBitsPerComponent is a hint to the function that describes the app's
// desired bit-depth of the render target. The function will attempt to create
// a render target with at least this many bits per component. For example, if 5
// is specified, the function might create a render target with a texel format
// of FTEX_FMT_R5G6B5. If 8 is specified, then FTEX_FMT_X8R8G8B8 might be used.
// Render targets don't have alpha information and so the alpha component of the
// texel format enum should be ignored. Pass in 0 for nDesiredBitsPerComponent to
// have the function automatically pick a texel format that matches the back
// buffer's pixel format.
//
// pszName is an optional parameter used to specify the created texture's name.
// If the parameter is not provided or if NULL is provided, the default texture
// name of FTEX_DEFAULT_RENDER_TARGET_NAME is used.

//FULLSCREEN API.
typedef void (*ftex_RenderTarget_Callback)(void *pUserData);

typedef enum
{
	FTEX_2D=0,
	FTEX_3D,
	FTEX_CUBE,

	FTEX_NUM_TYPES
} FTextureType_e;

//ARG - >>>>>
#if FANG_PLATFORM_PS2
extern CFTexInst *ftex_CreateRenderTarget( u32 nTexelsAcross, u32 nTexelsDown, FTexRenderTargetFmt_e nFormat, cchar *pszName, BOOL bPow2=TRUE, FResHandle_t hRes=FRES_NULLHANDLE, FResHandle_t *phRes=NULL, FTextureType_e nType=FTEX_2D, BOOL bDoubleBuffer=TRUE, BOOL bFullScreen=FALSE );
#else
//ARG - <<<<<
extern CFTexInst *ftex_CreateRenderTarget( u32 nTexelsAcross, u32 nTexelsDown, FTexRenderTargetFmt_e nFormat, cchar *pszName, BOOL bPow2=TRUE, FResHandle_t hRes=FRES_NULLHANDLE, FResHandle_t *phRes=NULL, FTextureType_e nType=FTEX_2D, BOOL bDoubleBuffer=TRUE );
#endif	//ARG
extern CFTexInst *ftex_CreateRenderTarget_FullScreen( FTexRenderTargetFmt_e nFormat, cchar *pszName, BOOL bPow2=FALSE, FResHandle_t hRes=FRES_NULLHANDLE, FResHandle_t *phRes=NULL, u32 nTexelsAcross=0, u32 nTexelsDown=0, FTextureType_e nType=FTEX_2D, BOOL bDoubleBuffer=TRUE );

extern void ftex_CreateSharedDepthBuffer(u8 nBPP, u32 nWidth, u32 nHeight);
extern void ftex_ClearSharedDepthBuffer();

extern CFTexInst *ftex_CreateDynamicEMBM( u32 nTexelsAcross, u32 nTexelsDown, cchar *pszName, FResHandle_t hRes=FRES_NULLHANDLE, FResHandle_t *phRes=NULL );
extern void ftex_HeightMapToEMBM(CFTexInst *pTex, f32 *pfData, BOOL bVec3=FALSE);

//
//Add RenderTarget to list, will be updated automatically based on settings and will call pCallback for rendering.
//bRenderWorld = render world once render target is rendered (FALSE for fullscreen FX)
//nMaxRenderFreq = maximum frequency (FPS) for rendering, 30 = 30 fps, 60 = fps ... Also if you want it to render every frame use a value of 0.
//bAutoClear??? = clear viewport after rendertarget is finished.
extern BOOL ftex_AddRenderTarget(CFTexInst *pTexInst, ftex_RenderTarget_Callback pCallback, BOOL bRenderWorld=TRUE, u32 nMaxRenderFreq=30, BOOL bAutoClearColor=TRUE, BOOL bAutoClearDepth=TRUE, void *pUserData=NULL, BOOL bAllowLiquid=FALSE, BOOL bForceNoLag=FALSE);

//This will force the current frame to be rendered and no buffer swap which may cause a stall for one frame but it will make sure that you don't get an old texture.
extern void ftex_FlushRenderTarget(CFTexInst *pTexInst);
extern void ftex_ActivateRenderTarget(CFTexInst *pTexInst, BOOL bActive);
extern void ftex_SetUserData(CFTexInst *pTexInst, void *pUserData);
extern void ftex_RenderTarget_SetAlphaTex( CFTexInst *pTexInst, CFTexInst *pAlphaTex );

extern BOOL ftex_HandleRenderTargets();
extern BOOL ftex_GetRenderWorldFlag();

extern void ftex_ClearRenderTargets();

//This allows staggered rendering of a set of render targets. So if you have 4 render targets updating at the same framerate, you can set it up so that
//on frame 0 only the first one is rendered and on frame 1 only the second is rendered, etc.
extern void ftex_StaggerRenderTarget(u8 nStaggerID=1);

extern void ftex_SetTexAddress( u32 nStageNum, BOOL bWrapU, BOOL bWrapV, BOOL bWrapW=TRUE );

extern void ftex_ReleaseTex( CFTexInst *pTex );

// This function allows the app to specify a new reflection map prior to any mesh load
// Any instances of pszOldReflectMap will be swapped for pszNewReflectMap at load time
FINLINE void ftex_SetLevelReflectionMap( cchar *pszOldReflectMap, cchar *pszNewReflectMap )
{
	fclib_strcpy( FTex_szOldReflectMap, pszOldReflectMap );
	fclib_strcpy( FTex_szNewReflectMap, pszNewReflectMap );
}

#endif

