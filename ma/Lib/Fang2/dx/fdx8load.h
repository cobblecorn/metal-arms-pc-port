//////////////////////////////////////////////////////////////////////////////////////
// fdx8load.h - Fang DX module responsible for converting data fresh off the disk
//              into the engine format.
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
// 10/22/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8LOAD_H_
#define _FDX8LOAD_H_ 1

#include "fang.h"

struct FMesh_t;

extern BOOL fdx8load_ModuleStartup( void );
extern void fdx8load_ModuleShutdown( void );

extern FMesh_t *fdx8load_Create( FMesh_t *pLoadMesh, cchar *pszResName );
extern void fdx8load_Destroy( FMesh_t *pMesh );


#endif

