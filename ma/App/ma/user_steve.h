//////////////////////////////////////////////////////////////////////////////////////
// user_steve.h - 
//
// Author: Michael Starich   
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
// 01/23/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _USER_STEVE_H_
#define _USER_STEVE_H_ 1

#include "fang.h"



class CEntity;
#include "fcoll.h"


extern BOOL user_steve_PreworldInit( void );
extern BOOL user_steve_PrelevelInit( void );
extern BOOL user_steve_PostlevelInit( void );
extern void user_steve_PrelevelShutdown( void );
extern BOOL user_steve_Main( void );
extern void user_steve_Terminate( void );
extern void user_steve_Work( void );
extern void user_steve_Draw( void );

extern void user_steve_Hit( CEntity *pEntity, const FCollImpact_t *pImpact, const CFVec3A *pUnitFireDir_WS );



#endif

