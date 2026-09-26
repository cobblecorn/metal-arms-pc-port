//////////////////////////////////////////////////////////////////////////////////////
// fGCviewport.h - Fang viewport module (GameCube version).
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

#ifndef _FGCVIEWPORT_H_
#define _FGCVIEWPORT_H_ 1

#include "fang.h"
#include "fGC.h"
#include "fviewport.h"
#include "flinklist.h"
#include "fmath.h"

//
typedef enum {
	FGCVIEWPORT_PROJTYPE_ORTHO = 0,
	FGCVIEWPORT_PROJTYPE_PERSP,
} FGCViewportProjType_e;

enum {
	FGCVIEWPORT_FLAG_INITIALIZED 					= 0x01,	//This viewport is initialized
	FGCVIEWPORT_FLAG_COVERS_ENTIRE_RENDER_TARGET	= 0x02, //This viewport covers the entire render target
	FGCVIEWPORT_FLAG_DONT_APPLY_VIEWPORT_SCALE		= 0x04,	//Dont apply the GC viewport scale to this viewport
	
	FGCVIEWPORT_FLAG_NONE							= 0x00
};
			

typedef struct FGCViewportIS_s FGCViewportIS_t;


FCLASS_ALIGN_PREFIX class CFGCViewportIS  
{
	public:
	
		Mtx44 		GCProjMtx;					// GC projection matrix
		
		FLink_t 	Link;						// Link to other viewports in free or used lists

		u32			nFlags;						// See GCVIEWPORT_FLAG_* flags above...
		
//		BOOL 		bInitialized;				// TRUE if the viewport has been initialized
//		BOOL 		bCoversEntireRenderTarget;	// TRUE if the corners of the viewport cover the entire render target surface
		FViewport_t Viewport;					// Viewport data

		f32 		fProjScale;					// Scale factor that has been applied to entire projection matrix
		
		FGCViewportProjType_e nMtxType;			// Type of projection matrix

	FCLASS_STACKMEM_ALIGN( CFGCViewportIS );
	
} FCLASS_ALIGN_SUFFIX;


extern BOOL fgcviewport_ModuleStartup( void );
extern void fgcviewport_ModuleShutdown( void );

extern void fgcviewport_RestoreState( void );
extern const Mtx44* fgcviewport_GetGCMatrix( const FViewport_t *pViewport );


#endif

