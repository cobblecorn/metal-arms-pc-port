//////////////////////////////////////////////////////////////////////////////////////
// guid.h - Game GUID generator.
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
// 04/11/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GUID_H_
#define _GUID_H_ 1

#include "fang.h"
#include "fguid.h"


#define GUID_NEW ::Guid.GetNew()


extern CFGuid Guid;


extern BOOL guid_InitSystem( void );
extern void guid_UninitSystem( void );
extern void guid_Reset( void );



#endif

