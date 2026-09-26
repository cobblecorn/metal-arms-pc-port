//////////////////////////////////////////////////////////////////////////////////////
// fdx8viewport.h - Fang viewport module (DX8 version).
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
// 10/03/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8VIEWPORT_H_
#define _FDX8VIEWPORT_H_ 1

#include "fang.h"
#include "fdx8.h"
#include "fviewport.h"
#include "fmath.h"




FCLASS_ALIGN_PREFIX class CFDX8ViewportIS {
public:
	CFMtx44A D3DProjMtx;				// D3D projection matrix

	BOOL bInitialized;					// TRUE if the viewport has been initialized
	BOOL bCoversEntireRenderTarget;		// TRUE if the corners of the viewport cover the entire render target surface
	FViewport_t Viewport;				// Viewport data

	f32 fProjScale;						// Scale factor that has been applied to entire projection matrix

	FCLASS_STACKMEM_ALIGN( CFDX8ViewportIS );
} FCLASS_ALIGN_SUFFIX;


extern BOOL fdx8viewport_ModuleStartup( void );
extern void fdx8viewport_ModuleShutdown( void );

extern void fdx8viewport_RestoreState( void );


#endif

