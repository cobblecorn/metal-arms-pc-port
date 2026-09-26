//////////////////////////////////////////////////////////////////////////////////////
// user_steve.cpp - 
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
// 01/23/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"

#if !FANG_PRODUCTION_BUILD

#include "user_steve.h"
#include "fviewport.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fxfm.h"
#include "fvid.h"
#include "fcolor.h"
#include "ftext.h"
#include "gamepad.h"
#include "floop.h"
#include "fanim.h"
#include "fverlet.h"
#include "fmotion.h"
#include "meshtypes.h"
#include "fworld_coll.h"
#include "meshentity.h"
#include "level.h"
#include "fparticle.h"
#include "fresload.h"


static FParticle_DefHandle_t _hPart;


BOOL user_steve_PreworldInit( void ) {
	return TRUE;
}

BOOL user_steve_PrelevelInit( void ) {
	return TRUE;
}


BOOL user_steve_PostlevelInit( void ) {
//	_hPart = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, "meinsteam" );

//	FParticle_EmitterHandle_t hPartEmitter = fparticle_SpawnEmitter( _hPart, CFVec3A( -143.0f, 0.0f, 206.0f ).v3, &CFVec3A::m_UnitAxisY.v3, 1.0f );
//	fparticle_EnableBurstSound( hPartEmitter, TRUE );

	return TRUE;
}


void user_steve_PrelevelShutdown( void ) {
}


BOOL user_steve_Main( void ) {
	return TRUE;
}


void user_steve_Terminate( void ) {
}


void user_steve_Work( void ) {
}


void user_steve_Draw( void ) {
}


void user_steve_Hit( CEntity *pEntity, const FCollImpact_t *pImpact, const CFVec3A *pUnitFireDir_WS ) {
}



#endif

