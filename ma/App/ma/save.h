//////////////////////////////////////////////////////////////////////////////////////
// save.h - Game save module.
//
// Author: Steve Ranck     
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
// 04/13/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SAVE_H_
#define _SAVE_H_ 1

#if 0
#include "fang.h"


#define SAVE_RESTYPE	"SAVE"


extern BOOL save_InitSystem( void );
extern void save_UninitSystem( void );

extern BOOL save_AllocateBuffer( u32 nByteCount );

extern void save_BeginSaveMode( void );
extern void save_EndSaveMode( void );
extern BOOL save_SaveObjectState( const void *pObjectAddress, u32 nObjectByteCount );

extern void save_RestoreAllObjects( void );

#endif

#endif

