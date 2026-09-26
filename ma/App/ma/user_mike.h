//////////////////////////////////////////////////////////////////////////////////////
// user_mike.h - 
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
#ifndef _USER_MIKE_H_
#define _USER_MIKE_H_ 1

#include "fang.h"
#include "sas_user.h"

#if SAS_ACTIVE_USER == SAS_USER_MIKE
	#define TESTBOTS_ENABLE		TRUE
#endif


extern BOOL user_mike_PreworldInit( void );
extern BOOL user_mike_PrelevelInit( void );
extern BOOL user_mike_Main( void );
extern void user_mike_Terminate( void );



#endif

