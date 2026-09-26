//////////////////////////////////////////////////////////////////////////////////////
// fdx8force.h - DX8 force feedback system to handle high-level waveforms.
//
// Author: John Lafleur
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
// 06/21/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fforce.h"
#include "fpadio.h"


static BOOL _bModuleInitialized;


BOOL fforce_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bModuleInitialized = TRUE;

	return TRUE;
}

void fforce_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}


//	fforce_Reset() - Makes sure all controllers have force-feedback
//	turned off and clears the envelope pool.
void fforce_Reset( void ) 
{
	FASSERT( _bModuleInitialized );

	return;
}

f32 fforce_SetMasterIntensity( u32 nDeviceIndex, f32 fUnitIntensity ) {
	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex >= 0 && nDeviceIndex < FPADIO_MAX_DEVICES );

	return 0.0f;
}

f32 fforce_GetMasterIntensity( u32 nDeviceIndex ) {
	FASSERT( _bModuleInitialized );

	return 0.0f;
}

void fforce_Work( void ) 
{
	FASSERT( _bModuleInitialized );

	return;
}

BOOL fforce_Play( u32 nDeviceIndex, FForce_Effect_e nEffect, FForceHandle_t *phDestHandle, FForceDomain_e nDomain/*=FFORCE_DOMAIN_NORMAL*/ )
{
	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex >= 0 && nDeviceIndex < FPADIO_MAX_DEVICES );
	FASSERT( nEffect > FFORCE_EFFECT_NONE && nEffect < FFORCE_MAX_FEEDBACK_EFFECTS );

	return FALSE;
}

void fforce_Kill( FForceHandle_t *phHandle )
{
	return;
};