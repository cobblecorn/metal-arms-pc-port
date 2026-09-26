//////////////////////////////////////////////////////////////////////////////////////
// LightPool.h - Light pool wrapper.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/28/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _LIGHTPOOL_H_
#define _LIGHTPOOL_H_ 1

#include "fang.h"
#include "flightpool.h"





FCLASS_ALIGN_PREFIX class CLightPool {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;
	

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }

	static FINLINE BOOL IsCreated( void ) { FASSERT(m_bSystemInitialized); return (BOOL)FLightPool_pPool; }

	static CFWorldLightItem *GetFromFreePool( void );
	static void ReturnToFreePool( CFWorldLightItem *pWorldLightItem );
	static void ReturnAllToFreePool( void );


	FCLASS_STACKMEM_ALIGN( CLightPool );
} FCLASS_ALIGN_SUFFIX;




#endif


