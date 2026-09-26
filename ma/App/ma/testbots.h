//////////////////////////////////////////////////////////////////////////////////////
// testbots.h - playground for test bots
//
// Author: Mike Elliott     
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
// 12/11/02 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////


#ifndef _TESTBOTS_H_
#define _TESTBOTS_H_ 1

#include "fang.h"

class CPlayer;
class CFMtx43A;

#ifndef TESTBOTS_ENABLE
	#define TESTBOTS_ENABLE (1 && !FANG_PRODUCTION_BUILD)
#endif

#if TESTBOTS_ENABLE
	extern void testbots_Init( void );
	extern void testbots_Work( void );
	extern void testbots_Draw( s32 nPlayer );
	extern void testbots_Cleanup( void );
	extern BOOL testbots_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
#else
	#define testbots_Init()				
	#define testbots_Work()				
	#define testbots_Draw(x)				
	#define testbots_Cleanup()			
	#define testbots_OverridePlayerBot(x,y)	FALSE	
#endif


#endif //TESTBOTS_ENABLE