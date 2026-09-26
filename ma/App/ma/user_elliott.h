//////////////////////////////////////////////////////////////////////////////////////
// user_pat.h - 
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
#ifndef _USER_ELLIOTT_H_
#define _USER_ELLIOTT_H_ 1

#include "fang.h"
#include "sas_user.h"

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT


#define TESTBOTS_ENABLE 1
extern BOOL user_elliott_PreworldInit( void );
extern BOOL user_elliott_PrelevelInit( void );
extern BOOL user_elliott_Main( void );
extern void user_elliott_Terminate( void );

#endif

#endif

