//////////////////////////////////////////////////////////////////////////////////////
// fGCsh.h - Fang GameCube shader module.
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
// 02/18/02	Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGCSH_H_
#define _FGCSH_H_ 1

#include "fang.h"
#include "fsh.h"

extern BOOL fgcsh_bWorldGeo;
extern BOOL fgcsh_bColorKey;

extern f32 fgcsh_fProjOffs;
extern f32 fgcsh_fProjRngAdj;

extern void fgcsh_Light_Activate( BOOL bUseVertexColor, BOOL bActivateChannel2=FALSE );
extern void fgcsh_Light_SetAmbient( u8 nRed, u8 nGreen, u8 nBlue, u8 nAlpha );
extern BOOL fgcsh_Execute( s32 nShaderIdx, u32 *pnShaderRegisters, BOOL bHonorCache = TRUE );
extern void fgcsh_SetAmbientLight(float r, float g, float b, float a);
extern void fgcsh_Swap( void );

extern void fgcsh_ResetSwapTables( void );
extern void fgcsh_SetCurrentPosMtx( u32 nMatrixIdx );
extern void fgcsh_SetCurrentNrmlMtxIdx( u32 nMatrixIdx );
extern void fgcsh_SetLMapTCStart(u32 nStart);

FINLINE void fgcsh_SetWorldGeoFlag(BOOL bWorldGeo) { fgcsh_bWorldGeo = bWorldGeo; }
FINLINE void fgcsh_EnableColorKey(BOOL bEnable) { fgcsh_bColorKey = bEnable; }

//****************GC Hardware perspective texcoord bug*****************
//On GC hardware, per-pixel perspective projection for texcoords, (s,t,1,q)
//the projected coordinate for a given pixel is s' = s/q and t' = t/q.
//The problem is that q cannot be negative so q is clamped to zero. 
//In a case where an edge of a triangle crosses the q = 0 plane, the interpolation along that
//edge will be incorrect since q's less the zero cannot be represented. 
//As a result a projection can become skewed in seamingly random ways.
//In order to solve this, we offset our projector and setup a virtual near plane
//where the projector originaly was. If the projector offset is great enough then no visible
//edge will cross the real plane and virtual plane, thus fixing the problem. Here are the functions for this:

//Sets the offset for projectors so that polygons cannot straddle both the real projector near plane and
//the virtual projector near plane (which is located at the original light position).
//This should only be non-zero if the level has a problem, and then it should be no bigger then the size of the
//biggest polygon effected by projectors.
FINLINE void fgcsh_SetProjectorOffset(f32 fProjOffs) { fgcsh_fProjOffs = fProjOffs; }

//When offseting a projector, if no change is made then the apparent FOV will change. To correct this, 
//the projector has a 'virtual' FOV so that the real projector cone and the virtual projector cone
//match at the projector range. The problem with this is that the light will appear too wide at the virtual
//near plane. The adjustment allows this FOV matching to happen closer to the near plane, thus making the 
//light thinner at the near plane but also thinner at the far plane. 
//1.0 = correction at far plane
//LightRange = correction at near plane.
//Use some adjustment in between (its clamped to the light range).
FINLINE void fgcsh_SetProjectorRangeAdj(f32 fRangeAdj) { fgcsh_fProjRngAdj = fRangeAdj; }

#endif
