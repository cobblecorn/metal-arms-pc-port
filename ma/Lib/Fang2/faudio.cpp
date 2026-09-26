//////////////////////////////////////////////////////////////////////////////////////
// faudio.cpp - Fang audio. - finally busting out some of the common functions into 1 file
//
// Author: Michael Starich
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
// 05/21/03 MStarich    Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "faudio.h"


f32 faudio_SetSfxMasterVol( f32 fNewVol ) {
	f32 fPrevious = FAudio_fMasterSfxUnitVol;
	
	FMATH_CLAMP_UNIT_FLOAT( fNewVol );
	FAudio_fMasterSfxUnitVol = fNewVol;
	
	if( FAudio_fMasterSfxUnitVol != fPrevious ) {
		FAudio_bMasterSfxVolChanged = TRUE;			
	}
	return fPrevious;
}


f32 faudio_GetSfxMasterVol( void ) {
	return FAudio_fMasterSfxUnitVol;
}


f32 faudio_SetMusicMasterVol( f32 fNewVol ) {
	f32 fPrevious = FAudio_fMasterMusicUnitVol;

	FMATH_CLAMP_UNIT_FLOAT( fNewVol );
	FAudio_fMasterMusicUnitVol = fNewVol;

	if( FAudio_fMasterMusicUnitVol != fPrevious ) {
		FAudio_bMasterMusicVolChanged = TRUE;			
	}
	return fPrevious;	
}


f32 faudio_GetMusicMasterVol( void ) {
	return FAudio_fMasterMusicUnitVol;
}


BOOL faudio_IsInstalled( void ) {
	return FAudio_bModuleInstalled;
} // faudio_IsInstalled


void faudio_GetBankAttributes( FAudio_BankHandle_t oBankHandle, FAudio_BankInfo_t *poBankInfo ) {
	
	if( ! FAudio_bModuleInstalled ) {
		return;
	}

	FASSERT_MSG( faudio_IsValidBankHandle( oBankHandle ), "[ FAUDIO ] Error: Invalid handle !!!" );
	FASSERT_MSG( poBankInfo,                              "[ FAUDIO ] Error: NULL pointer !!!" );

	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)oBankHandle;

	fclib_strcpy( poBankInfo->szName, poBank->szName );
	poBankInfo->uWaves = poBank->uWaves;
} // faudio_GetBankAttributes


FAudio_WaveHandle_t faudio_GetWaveHandle( FAudio_BankHandle_t oBankHandle, cchar *pszName ) {

	if( ! FAudio_bModuleInstalled ) {
		return FAUDIO_INVALID_HANDLE;
	}
	
	if( !faudio_IsValidBankHandle( oBankHandle ) ) {
		DEVPRINTF( "faudio_GetWaveHandle() - Error: Invalid handle !!!" );
		return FAUDIO_INVALID_HANDLE;
	}

	FASSERT_MSG( pszName,                                                     "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pszName,                                                    "[ FAUDIO ] Error: Zero length string !!!" );
	FASSERT_MSG( ( FAUDIO_MAX_ASSET_NAME_LENGTH >= fclib_strlen( pszName ) ), "[ FAUDIO ] Error: Invalid bank name !!!" );

	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)oBankHandle;
	FDataWvbFile_Wave_t *poWave = (FDataWvbFile_Wave_t *)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) );

	for( u32 uIndex = 0; uIndex < poBank->uWaves; ++uIndex, ++poWave ) {
		FASSERT_MSG( poWave->szName,  "[ FAUDIO ] Error: NULL pointer !!!" );
		FASSERT_MSG( *poWave->szName, "[ FAUDIO ] Error: Zero length string !!!" );

		if( 0 == fclib_stricmp( poWave->szName, pszName ) ) {
			return (FAudio_WaveHandle_t)poWave;
		}
	}

	return FAUDIO_INVALID_HANDLE;
} // faudio_GetWaveHandle


FAudio_WaveHandle_t faudio_GetWaveHandle( FAudio_BankHandle_t oBankHandle, u32 uWaveIndex )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_INVALID_HANDLE;
	}

	FASSERT_MSG( faudio_IsValidBankHandle( oBankHandle ), "[ FAUDIO ] Error: Invalid handle !!!" );

	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)( oBankHandle );

	FASSERT_MSG( ( poBank->uWaves > uWaveIndex ), "[ FAUDIO ] Error: uWaveIndex invalid !!!" );

	return (FAudio_WaveHandle_t)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) + ( sizeof( FDataWvbFile_Wave_t ) * uWaveIndex ) );
} // faudio_GetWaveHandle


void faudio_GetWaveAttributes( FAudio_BankHandle_t oBankHandle, u32 uWaveIndex, FAudio_WaveInfo_t *poWaveInfo ) {

	if( ! FAudio_bModuleInstalled ) {
		return;
	}

	FASSERT_MSG( faudio_IsValidBankHandle( oBankHandle ), "[ FAUDIO ] Error: Invalid handle !!!" );
	FASSERT_MSG( poWaveInfo,                              "[ FAUDIO ] Error: NULL pointer !!!" );

	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)( oBankHandle );

	FASSERT_MSG( ( poBank->uWaves > uWaveIndex ), "[ FAUDIO ] Error: uWaveIndex invalid !!!" );

	FDataWvbFile_Wave_t *poWave = (FDataWvbFile_Wave_t *)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) + ( sizeof( FDataWvbFile_Wave_t ) * uWaveIndex ) );

	fclib_strcpy( poWaveInfo->szName, poWave->szName );
	poWaveInfo->uChannels        = poWave->nNumChannels;
	poWaveInfo->fFrequency       = poWave->fFreqHz;
	poWaveInfo->fLengthInSeconds = poWave->fLengthInSeconds;
} // faudio_GetWaveAttributes


void faudio_GetWaveAttributes( FAudio_WaveHandle_t oWaveHandle, FAudio_WaveInfo_t *poWaveInfo ) {

	if( ! FAudio_bModuleInstalled ) {
		return;
	}

	FASSERT_MSG( faudio_IsValidWaveHandle( oWaveHandle ), "[ FAUDIO ] Error: Invalid handle !!!" );
	FASSERT_MSG( poWaveInfo,                              "[ FAUDIO ] Error: NULL pointer !!!" );

	FDataWvbFile_Wave_t *poWave = (FDataWvbFile_Wave_t *)oWaveHandle;
	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)( poWave->oBankHandle );

	fclib_strcpy( poWaveInfo->szName, poWave->szName );

	poWaveInfo->uChannels  = poWave->nNumChannels;
	poWaveInfo->fFrequency = poWave->fFreqHz;

	poWaveInfo->fLengthInSeconds = poWave->fLengthInSeconds;
} // faudio_GetWaveAttributes





