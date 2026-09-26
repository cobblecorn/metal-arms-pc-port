//////////////////////////////////////////////////////////////////////////////////////
// fdxsh_psprite.vsh - Fang point sprite vertex shader.
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
// 06/01/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

vs.1.0

//------------------------------------------------------------------------------
// Vertex Streams:
//   v0: Position in model space (x,y,z,1)
//   v1: Dimension in model units (d,0,0,1)
//   v2: Color (r,g,b,a)
//
// Inputs:
//   c[0-3]: Projection * View * Model matrix
//   c[4].x: Projection point scale factor 1
//   c[4].y: Projection point scale factor 2
//   c[4].z: -1/(FogEndZ - FogStartZ)
//   c[4].w: FogEndZ/(FogEndZ - FogStartZ)
//------------------------------------------------------------------------------

m4x4	r0, v0, c0					// Transform vertex into view space
mov		oD0, v2						// Output diffuse color
rcp		r2.x, r0.w					// Compute 1/z (note that this is scaled by the projection matrix)
mov		oPos, r0					// Output position in clip space
mad		r3.x, r2.x, c4.x, c4.y		// Compute scale multiplier
mad		oFog.x, r0.z, c4.z, c4.w	// Output fog factor
mul		oPts.x, v1.x, r3.x			// Compute scale and output in screen space

