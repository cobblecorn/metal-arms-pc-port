//////////////////////////////////////////////////////////////////////////////////////
// Splat.h - Splat SFX class, for flat textures mapped to the ground
//
// Author: Mike Elliott
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
// 11.18.02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef _SPLAT_H_
#define _SPLAT_H_ 1

#include "fang.h"
#include "fdraw.h"
#include "ftex.h"

#ifndef _MMI_TARGET_PS2	//ARG
FCLASS_NOALIGN_PREFIX class CSplat {
//ARG - >>>>>
#else
FCLASS_ALIGN_PREFIX class CSplat {
#endif
//ARG - <<<<<

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef enum {
		SPLATTYPE_SQUISHED_GLITCH,
		SPLATTYPE_SQUISHED_GRUNT,

		SPLATTYPE_COUNT
	} SplatType_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL			m_bSystemInitialized;				// TRUE if the system has been initialized
	static BOOL			m_bSystemActive;
	static CSplat	   *m_aSplats;
	static CFTexInst	m_aTextures[SPLATTYPE_COUNT];		// one tex for each type of splat
	static cchar	   *m_apszSquishedTexNames[];


	
	SplatType_e  m_Type;
	FDrawVtx_t	*m_aVtx;

	BOOL	m_bActive;		// should this splat do work/be drawn
	f32		m_fSize;		// how big per side
	u64		m_uTime;		// time this splat was created
	CFVec3	m_vPos;			// where the splat is




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; };

	static void Work( void );
	static void Draw( void );

	static void KillAll( void );

	// for now, they're all 5 verts per side, may want to dynamically lower based on vertical distance between corners
	static void AddSplat( u32 uType, f32 fSize, const CFVec3A &vPos, const CFVec3A &vFront );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	CSplat();

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL		_InitTextures( void );
	static CSplat  *_GetNewSplat( void );		//returns a splat to be replaced

	void _Work( void );
	void _Draw( void );
	BOOL _IsOffscreen( void );
	

	


#ifndef _MMI_TARGET_PS2	//ARG
FCLASS_STACKMEM_NOALIGN( CSplat );
} FCLASS_NOALIGN_SUFFIX;
//ARG - >>>>>
#else
FCLASS_STACKMEM_ALIGN( CSplat );
} FCLASS_ALIGN_SUFFIX;
#endif
//ARG - <<<<<

#endif

#endif
