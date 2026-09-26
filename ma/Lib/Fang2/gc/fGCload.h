//////////////////////////////////////////////////////////////////////////////////////
// fGCload.h - Fang GameCube module responsible for converting data fresh off the disk
//              into the engine format.
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

#ifndef _FGCLOAD_H_
#define _FGCLOAD_H_ 1

#include "fang.h"
#include "fdata.h"
#include "fmesh.h"


extern BOOL fgcload_ModuleStartup( void );
extern void fgcload_ModuleShutdown( void );

extern FMesh_t *fgcload_Create( FMesh_t *pLoadMesh, cchar *pszResName );
extern void fgcload_Destroy( FMesh_t *pMesh );


#endif

