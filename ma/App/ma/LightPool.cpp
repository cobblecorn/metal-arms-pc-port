//////////////////////////////////////////////////////////////////////////////////////
// LightPool.cpp - Light pool wrapper.
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
#include "fang.h"
#include "lightpool.h"
#include "flightpool.h"


BOOL CLightPool::m_bSystemInitialized;



BOOL CLightPool::InitSystem( void ) {
	FASSERT( !IsSystemInitialized() );

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CLightPool::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;
	}
}


CFWorldLightItem *CLightPool::GetFromFreePool( void ) {
	FASSERT( IsSystemInitialized() );

	if( !IsCreated() ) {
		return NULL;
	}

	return FLightPool_pPool->GetFromFreePool();
}


void CLightPool::ReturnToFreePool( CFWorldLightItem *pWorldLightItem ) {
	FASSERT( IsSystemInitialized() );

	if( !IsCreated() ) {
		return;
	}

	FLightPool_pPool->ReturnToFreePool( pWorldLightItem );
}


void CLightPool::ReturnAllToFreePool( void ) {
	FASSERT( IsSystemInitialized() );

	if( !IsCreated() ) {
		return;
	}

	FLightPool_pPool->ReturnAllToFreePool();
}



