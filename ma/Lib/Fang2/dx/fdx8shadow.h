//////////////////////////////////////////////////////////////////////////////////////
// fdx8shadow.h - 
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
// 07/10/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8SHADOW_H_
#define _FDX8SHADOW_H_ 1

#include "fang.h"

extern BOOL fdx8shadow_ModuleStartup( void );
extern void fdx8shadow_ModuleShutdown( void );

extern void fdx8shadow_SetupVertexShaders();
extern void fdx8shadow_SetupPixelShaders();

extern void fdx8shadow_DeleteVertexShaders();
extern void fdx8shadow_DeletePixelShaders();

extern void fdx8shadow_ClearTextures();

#endif

