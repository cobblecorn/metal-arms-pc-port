//////////////////////////////////////////////////////////////////////////////////////
// fsndfx.cpp - 
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
// 06/19/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fsndfx.h"
#include "fclib.h"
#include "fresload.h"

//====================
// private definitions

#define _MAX_LOADED_SFX_BANKS						32

#define _CONVERT_OFFSET_TO_PTR( nOffset, nBase )	( (u32)nOffset + (u32)nBase )

// TO PREVENT VALID FX HANDLES FROM EVER BEING ZERO,
// HERE IS THE FORMAT OF AN FX HANDLE: 
// HI 8 BITS == BANK INDEX
// BITS 20 THRU 23 ALL SET
// LO 20 BITS == FX INDEX
#define _HANDLE_SIGNATURE							0x00F00000
#define _CREATE_FX_HANDLE( nBankIndex, nFxIndex )	(FSndFx_FxHandle_t)( ( ( ((u32)nBankIndex) & 0x000000FF ) << 24 ) | ( ((u32)nFxIndex) & 0x000FFFFF ) | _HANDLE_SIGNATURE ) 
#define _GET_BANK_ID_FROM_HANDLE( hHandle )			(u32)( ((u32)hHandle) >> 24 )
#define _GET_FX_INDEX_FROM_HANDLE( hHandle )		(u32)( ((u32)hHandle) & 0x000FFFFF )

#if FANG_PLATFORM_WIN
	// only allowed under windows
	#define _PRINT_ALL_PLAYED_SOUNDS				FALSE// TRUE, prints all played sound calls to a text file
#else
	// never allowed under anything but windows
	#define _PRINT_ALL_PLAYED_SOUNDS				FALSE
#endif

//=================
// public variables

//==================
// private variables

static BOOL _bModuleInited = FALSE;

static u32 _nNumLoadedBanks = 0;
static FData_SFxBank_Header_t *_apBanks[_MAX_LOADED_SFX_BANKS];

static FResLoadReg_t _ResLoadRegistration;

#if _PRINT_ALL_PLAYED_SOUNDS
	#include "ffile.h"
	static FFileHandle _hLogFileHandle = FFILE_INVALID_HANDLE;
#endif

//===================
// private prototypes

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _ResLoadDestroy( void *pBase );
static FData_SFxBank_Seq_t *_ConvertHandleToSeqPtr( FSndFx_FxHandle_t hHandle );

//=================
// public functions

BOOL fsndfx_ModuleStartup( void ) {

	// register our fres loader function
	fres_CopyType( _ResLoadRegistration.sResType, FSNDFX_RESTYPE );
	_ResLoadRegistration.pszFileExtension = "sfb";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_PERM;
	_ResLoadRegistration.nAlignment = 4;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = _ResLoadDestroy;

	if( !fresload_RegisterHandler( &_ResLoadRegistration ) ) {
		// Registration failed...
		DEVPRINTF( "fsndfx_ModuleStartup(): Could not register resource, no sound fxs are possible.\n" );
		return FALSE;
	}

	_nNumLoadedBanks = 0;
	
	u32 i;
	for( i=0; i < _MAX_LOADED_SFX_BANKS; i++ ) {
		_apBanks[i] = NULL;
	}

	_bModuleInited = TRUE;

#if _PRINT_ALL_PLAYED_SOUNDS
	_hLogFileHandle = ffile_Open( "sound file log.txt", FFILE_OPEN_TRUNC_OR_CREATE_WONLY, TRUE );
#endif

	return TRUE;
}

void fsndfx_ModuleShutdown( void ) {
	
	_nNumLoadedBanks = 0;
	
	u32 i;
	for( i=0; i < _MAX_LOADED_SFX_BANKS; i++ ) {
		_apBanks[i] = NULL;
	}

	_bModuleInited = FALSE;

#if _PRINT_ALL_PLAYED_SOUNDS
	if( _hLogFileHandle != FFILE_INVALID_HANDLE ) {
        ffile_Close( _hLogFileHandle );
		_hLogFileHandle = FFILE_INVALID_HANDLE;
	}	
#endif
}

u32 fsndfx_GetNumLoadedBanks( void ) {

	if( !_bModuleInited ) {
		return 0;
	}
	return _nNumLoadedBanks;
}

FSndFx_BankHandle_t fsndfx_GetBankHandle( u32 nBankIndex ) {
	
	if( !_bModuleInited ||
		nBankIndex >= _nNumLoadedBanks ) {
		return FSNDFX_INVALID_BANK_HANDLE;
	}
	return (FSndFx_BankHandle_t)_apBanks[nBankIndex];
}

u32 fsndfx_GetNumFxsInBank( FSndFx_BankHandle_t hBank ) {
	
	if( !_bModuleInited ||
		hBank == FSNDFX_INVALID_BANK_HANDLE ||
		_nNumLoadedBanks == 0 ) {
		return 0;
	}
	// see if hBank is even loaded anymore
	FData_SFxBank_Header_t *pHeader = (FData_SFxBank_Header_t *)hBank;
	if( pHeader->nKey >= _MAX_LOADED_SFX_BANKS ||
		(u32)pHeader != (u32)_apBanks[pHeader->nKey] ) {
		// this is a valid bank handle
		return 0;
	}
	return pHeader->nNumSFx;
}

FSndFx_FxHandle_t fsndfx_GetFxHandle( FSndFx_BankHandle_t hBank, u32 nFxIndex ) {

	if( !_bModuleInited ||
		hBank == FSNDFX_INVALID_BANK_HANDLE ||
		_nNumLoadedBanks == 0 ) {
		return FSNDFX_INVALID_FX_HANDLE;
	}
	// see if hBank is even loaded anymore
	FData_SFxBank_Header_t *pHeader = (FData_SFxBank_Header_t *)hBank;
	if( pHeader->nKey >= _MAX_LOADED_SFX_BANKS ||
		(u32)pHeader != (u32)_apBanks[pHeader->nKey] ) {
		// this is a valid bank handle
		return FSNDFX_INVALID_FX_HANDLE;
	}
	if( nFxIndex >= pHeader->nNumSFx ) {
		return FSNDFX_INVALID_FX_HANDLE;
	}
	return pHeader->paSeqs[nFxIndex].hHandleToMyself;
}

FSndFx_FxHandle_t fsndfx_GetFxHandle( cchar *pszName ) {

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ||
		!pszName ) {
		return FSNDFX_INVALID_FX_HANDLE;
	}
	u32 i, j, nNumFxs;
	FData_SFxBank_Seq_t *pSeq;

	for( i=0; i < _nNumLoadedBanks; i++ ) {
		pSeq = (FData_SFxBank_Seq_t *)_apBanks[i]->paSeqs;

		nNumFxs = _apBanks[i]->nNumSFx;
		for( j=0; j < nNumFxs; j++ ) {	

			if( fclib_stricmp( pszName, pSeq[j].pszName ) == 0 ) {
				return pSeq[j].hHandleToMyself;
			}
		}
	}
	return FSNDFX_INVALID_FX_HANDLE;
}

cchar *fsndfx_ConvertFxHandleToName( FSndFx_FxHandle_t hHandle ) {

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return NULL;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return NULL;
	}
	return pSeq->pszName;
}

FAudio_WaveHandle_t fsndfx_GetWavHandle( FSndFx_FxHandle_t hHandle ) {

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return NULL;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return NULL;
	}

	FASSERT( pSeq->nNumCmds > 0 );
	FASSERT( pSeq->paCmds[0].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY );

	FData_SFxBank_PlayCmd_t *pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[0].pCmdData;

	return pPlay->WavHandle;
}

// returns -1 if there was a problem (like an invalid handle)
// otherwise, the number of loops will be returned, 0 is loop forever
s32 fsndfx_GetNumLoops( FSndFx_FxHandle_t hHandle ) {

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return -1;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return -1;
	}

	FASSERT( pSeq->nNumCmds > 0 );
	FASSERT( pSeq->paCmds[0].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY );

	FData_SFxBank_PlayCmd_t *pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[0].pCmdData;

	return pPlay->nLoops;
}

// Returns the duration in seconds of the specified sound,
// or 0.0f if the sound could not be determined for any reason.
f32 fsndfx_GetDuration( FSndFx_FxHandle_t hHandle ) {
	if( !_bModuleInited || (_nNumLoadedBanks == 0) || (hHandle == 0) ) {
		return 0.0f;
	}

	FAudio_WaveHandle_t hWave = fsndfx_GetWavHandle( hHandle );
	if( hWave == 0 ) {
		return 0.0f;
	}

	FAudio_WaveInfo_t WaveInfo;

	faudio_GetWaveAttributes( hWave, &WaveInfo );

	return WaveInfo.fLengthInSeconds;
}

f32 fsndfx_GetUnitPlayVolume( FSndFx_FxHandle_t hHandle ) {
	
	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return NULL;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return NULL;
	}

	FASSERT( pSeq->nNumCmds > 0 );
	FASSERT( pSeq->paCmds[0].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY );

	FData_SFxBank_PlayCmd_t *pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[0].pCmdData;

	return pPlay->fUnitVolume;
}

#if FANG_WINGC
// Port diagnostic (-port-diag): the first plays of each sound with its final volume, for mix work.
static void _PortLogPlay( const FData_SFxBank_Seq_t *pSeq, cchar *pszKind, f32 fVolume, f32 fRadius ) {
	static cchar *_apszSeen[512];
	static u8 _anSeen[512];
	static u32 _nSeen = 0;
	if( !Fang_bPortDiag ) {
		return;
	}
	u32 i;
	for( i=0; i < _nSeen && _apszSeen[i] != pSeq->pszName; i++ ) {
	}
	if( i == _nSeen ) {
		if( _nSeen >= 512 ) {
			return;
		}
		_apszSeen[_nSeen] = pSeq->pszName;
		_anSeen[_nSeen++] = 0;
	}
	if( _anSeen[i] < 3 ) {
		_anSeen[i]++;
		DEVPRINTF( "PORT-SND %s '%s' vol=%.2f radius=%.0f\n", pszKind, pSeq->pszName, fVolume, fRadius );
	}
}
#define _PORT_LOG_PLAY( pSeq, pszKind, fVolume, fRadius )	_PortLogPlay( pSeq, pszKind, fVolume, fRadius )
#else
#define _PORT_LOG_PLAY( pSeq, pszKind, fVolume, fRadius )
#endif

BOOL fsndfx_Play2D( FSndFx_FxHandle_t hHandle,
				    f32 fVolumeMultiplier/*=1.0f*/,
					f32 fFreqMultiplier/*=1.0f*/,
				    u8 nPriority/*=FAudio_EmitterDefaultPriorityLevel*/,
					f32 fPanLeftRight/*=0.0f*/,
					BOOL bDuckable/*=TRUE*/,
					CFAudioEmitter **ppUserAudioEmitter/*=NULL*/) {

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return FALSE;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return FALSE;
	}

#if _PRINT_ALL_PLAYED_SOUNDS
	if( _hLogFileHandle != FFILE_INVALID_HANDLE ) {
		ffile_Write( _hLogFileHandle, fclib_strlen( pSeq->pszName ), (void *)pSeq->pszName );	
		ffile_Write( _hLogFileHandle, 2, (void *)"\r\n" );
	}
#endif
		
	FASSERT( pSeq->nNumCmds > 0 );
	FASSERT( pSeq->paCmds[0].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY );

	FData_SFxBank_PlayCmd_t *pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[0].pCmdData;
	FASSERT( pPlay );

	_PORT_LOG_PLAY( pSeq, "2D", pPlay->fUnitVolume * fVolumeMultiplier, 0.0f );
	if( CFAudioEmitter::Play2D( pPlay->WavHandle,
								pPlay->fUnitVolume * fVolumeMultiplier,
								nPriority,
								bDuckable,
								pPlay->nLoops,
								fPanLeftRight,
								pPlay->fFrequencyFactor * fFreqMultiplier,
								ppUserAudioEmitter
							) != FAUDIO_NO_ERROR ) {
		return FALSE;
	}
	return TRUE;
}

BOOL fsndfx_Play3D( FSndFx_FxHandle_t hHandle,
					const CFVec3A *pPos_WS,
					f32 fOuterRadius, 
					f32 fSpawnRadiusFactor/*=-1.0f*/,
					f32 fVolumeMultiplier/*=1.0f*/,
					f32 fFreqMultiplier/*=1.0f*/,
					u8 nPriority/*=FAudio_EmitterDefaultPriorityLevel*/,
					BOOL bDuckable/*=TRUE*/,
					CFAudioEmitter **ppUserAudioEmitter/*=NULL*/) {

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return FALSE;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return FALSE;
	}

#if _PRINT_ALL_PLAYED_SOUNDS
	if( _hLogFileHandle != FFILE_INVALID_HANDLE ) {
		ffile_Write( _hLogFileHandle, fclib_strlen( pSeq->pszName ), (void *)pSeq->pszName );	
		ffile_Write( _hLogFileHandle, 2, (void *)"\r\n" );
	}
#endif
	
	FASSERT( pSeq->nNumCmds > 0 );
	FASSERT( pSeq->paCmds[0].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY );

	FData_SFxBank_PlayCmd_t *pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[0].pCmdData;
	FASSERT( pPlay );

	_PORT_LOG_PLAY( pSeq, "3D", pPlay->fUnitVolume * fVolumeMultiplier, fOuterRadius );
	if( CFAudioEmitter::Play3D( pPlay->WavHandle,
								pPos_WS,
								fOuterRadius,
								pPlay->fUnitVolume * fVolumeMultiplier,
								nPriority,
								bDuckable,
								pPlay->nLoops,
								pPlay->fFrequencyFactor * fFreqMultiplier,
								fSpawnRadiusFactor,
								FALSE,
								ppUserAudioEmitter
							) != FAUDIO_NO_ERROR ) {
		return FALSE;
	}
	return TRUE;
}

#if !FANG_PRODUCTION_BUILD
CFAudioEmitter *fsndfx_AllocNPlay2DSound( cchar *pszFileName, u32 nLineNum,
								     FSndFx_FxHandle_t hHandle,
									 f32 fVolumeMultiplier/*=1.0f*/,
									 f32 fFreqMultiplier/*=1.0f*/,
									 u8 nPriority/*=FAudio_EmitterDefaultPriorityLevel*/,
									 f32 fPanLeftRight/*=0.0f*/,
									 BOOL bDuckable/*=TRUE*/ ) {
#else
CFAudioEmitter *fsndfx_AllocNPlay2DSound( FSndFx_FxHandle_t hHandle,
									 f32 fVolumeMultiplier/*=1.0f*/,
									 f32 fFreqMultiplier/*=1.0f*/,
									 u8 nPriority/*=FAudio_EmitterDefaultPriorityLevel*/,
									 f32 fPanLeftRight/*=0.0f*/,
									 BOOL bDuckable/*=TRUE*/ ) {
#endif // !FANG_PRODUCTION_BUILD

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return NULL;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return NULL;
	}

#if _PRINT_ALL_PLAYED_SOUNDS
	if( _hLogFileHandle != FFILE_INVALID_HANDLE ) {
		ffile_Write( _hLogFileHandle, fclib_strlen( pSeq->pszName ), (void *)pSeq->pszName );	
		ffile_Write( _hLogFileHandle, 2, (void *)"\r\n" );
	}
#endif
	
	FASSERT( pSeq->nNumCmds > 0 );
	FASSERT( pSeq->paCmds[0].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY );

	FData_SFxBank_PlayCmd_t *pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[0].pCmdData;
	FASSERT( pPlay );

	CFAudioEmitter *pEmitter = CFAudioEmitter::Create2D( pPlay->WavHandle, nPriority );
	if( !pEmitter ) {
		return NULL;
	}

	// set the emitters parameters
	pEmitter->SetVolume( pPlay->fUnitVolume * fVolumeMultiplier );
	_PORT_LOG_PLAY( pSeq, "emitter", pPlay->fUnitVolume * fVolumeMultiplier, 0.0f );
	pEmitter->SetDuckable( bDuckable );
	pEmitter->SetPan( fPanLeftRight );
	pEmitter->SetFrequencyFactor( pPlay->fFrequencyFactor * fFreqMultiplier );

#if !FANG_PRODUCTION_BUILD
	pEmitter->m_pszModuleOwner = pszFileName;
	pEmitter->m_nOwnerModuleLine = nLineNum;
#endif

	pEmitter->Play( pPlay->nLoops );

	return pEmitter;
}

#if !FANG_PRODUCTION_BUILD
CFAudioEmitter *fsndfx_AllocNPlay3DSound( cchar *pszFileName, u32 nLineNum,
									 FSndFx_FxHandle_t hHandle,
									 const CFVec3A *pPos_WS,
									 f32 fOuterRadius, 
									 f32 fVolumeMultiplier/*=1.0f*/,
									 f32 fFreqMultiplier/*=1.0f*/,
									 u8 nPriority/*=FAudio_EmitterDefaultPriorityLevel*/,
									 BOOL bDuckable/*=TRUE*/ ) {
#else
CFAudioEmitter *fsndfx_AllocNPlay3DSound( FSndFx_FxHandle_t hHandle,
									 const CFVec3A *pPos_WS,
									 f32 fOuterRadius, 
									 f32 fVolumeMultiplier/*=1.0f*/,
									 f32 fFreqMultiplier/*=1.0f*/,
									 u8 nPriority/*=FAudio_EmitterDefaultPriorityLevel*/,
									 BOOL bDuckable/*=TRUE*/ ) {
#endif // !FANG_PRODUCTION_BUILD

	if( !_bModuleInited ||
		_nNumLoadedBanks == 0 ) {
		return NULL;
	}
	FData_SFxBank_Seq_t *pSeq = _ConvertHandleToSeqPtr( hHandle );
	if( !pSeq ) {
		return NULL;
	}

#if _PRINT_ALL_PLAYED_SOUNDS
	if( _hLogFileHandle != FFILE_INVALID_HANDLE ) {
		ffile_Write( _hLogFileHandle, fclib_strlen( pSeq->pszName ), (void *)pSeq->pszName );	
		ffile_Write( _hLogFileHandle, 2, (void *)"\r\n" );
	}
#endif
	
	FASSERT( pSeq->nNumCmds > 0 );
	FASSERT( pSeq->paCmds[0].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY );

	FData_SFxBank_PlayCmd_t *pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[0].pCmdData;
	FASSERT( pPlay );

	CFAudioEmitter *pEmitter = CFAudioEmitter::Create3D( pPlay->WavHandle, nPriority, pPos_WS, fOuterRadius );
	if( !pEmitter ) {
		return NULL;
	}

	// set the emitters parameters
	pEmitter->SetVolume( pPlay->fUnitVolume * fVolumeMultiplier );
	_PORT_LOG_PLAY( pSeq, "emitter", pPlay->fUnitVolume * fVolumeMultiplier, 0.0f );
	pEmitter->SetDuckable( bDuckable );
	pEmitter->SetFrequencyFactor( pPlay->fFrequencyFactor * fFreqMultiplier );
	
#if !FANG_PRODUCTION_BUILD
	pEmitter->m_pszModuleOwner = pszFileName;
	pEmitter->m_nOwnerModuleLine = nLineNum;
#endif

	pEmitter->Play( pPlay->nLoops );

	return pEmitter;
}

//==================
// private functions

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName ) {
	FData_SFxBank_Header_t *pHeader = (FData_SFxBank_Header_t *)pLoadedBase;

	if( !_bModuleInited ) {
		return FALSE;
	}
	if( !FAudio_bModuleInstalled ) {
		DEVPRINTF( "fsndfx _ResLoadCreate(): Skipping sound fx bank '%s' because audio is not installed.\n", pszResName );
		return FALSE;
	}

	if( _nNumLoadedBanks >= _MAX_LOADED_SFX_BANKS ) {
		DEVPRINTF( "fsndfx _ResLoadCreate(): Too many Sound Fx Banks loaded already, the current max is %d.\n", _MAX_LOADED_SFX_BANKS );
		return FALSE;
	}

	// fixup the ptrs
	u32 i, j, nBankIndex, uNumGoodSounds;
	FData_SFxBank_Seq_t *pSeq;
	FData_SFxBank_PlayCmd_t *pPlay;
	u32 nBaseAddress = (u32)pLoadedBase;
	pHeader->pszBankName = (cchar *)_CONVERT_OFFSET_TO_PTR( pHeader->pszBankName, nBaseAddress );
	pHeader->paSeqs = (FData_SFxBank_Seq_t *)_CONVERT_OFFSET_TO_PTR( pHeader->paSeqs, nBaseAddress );

	pSeq = pHeader->paSeqs;
	for( i=0; i < pHeader->nNumSFx; i++ ) {
		pSeq[i].pszName = (cchar *)_CONVERT_OFFSET_TO_PTR( pSeq[i].pszName, nBaseAddress );
		pSeq[i].paCmds = (FData_SFxBank_Cmd_t *)_CONVERT_OFFSET_TO_PTR( pSeq[i].paCmds, nBaseAddress );

		for( j=0; j < pSeq[i].nNumCmds; j++ ) {
			pSeq[i].paCmds[j].pCmdData = (void *)_CONVERT_OFFSET_TO_PTR( pSeq[i].paCmds[j].pCmdData, nBaseAddress );
			
			if( pSeq[i].paCmds[j].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY ) {
				pPlay = (FData_SFxBank_PlayCmd_t *)pSeq[i].paCmds[j].pCmdData;
		
				pPlay->pszWavName = (cchar *)_CONVERT_OFFSET_TO_PTR( pPlay->pszWavName, nBaseAddress );
			}
		}
	}

	// load the referenced wav bank
	pHeader->BankHandle = faudio_LoadBank( pHeader->pszBankName );
	if( pHeader->BankHandle == FAUDIO_INVALID_HANDLE ) {
		DEVPRINTF( "fsndfx _ResLoadCreate(): The sound fx bank '%s' could not load the referenced wav bank '%s'.\n", pszResName, pHeader->pszBankName );
		return FALSE;
	}
	
	// record where we will be putting the bank
	nBankIndex = _nNumLoadedBanks;

	// fixup some data based on the bank
	pHeader->nKey = nBankIndex;
	
	uNumGoodSounds = 0;
	
	for( i=0; i < pHeader->nNumSFx; i++ ) {
		pSeq[i].hHandleToMyself = _CREATE_FX_HANDLE( nBankIndex, i );

		for( j=0; j < pSeq[i].nNumCmds; j++ ) {
			if( pSeq[i].paCmds[j].nCmdType == FDATA_SFXBANK_CMD_TYPE_PLAY ) {
				pPlay = (FData_SFxBank_PlayCmd_t *)pSeq[i].paCmds[j].pCmdData;
				
				pPlay->WavHandle = faudio_GetWaveHandle( pHeader->BankHandle, pPlay->pszWavName );
				
				if( pPlay->WavHandle == FAUDIO_INVALID_HANDLE ) {
					DEVPRINTF( "fsndfx _ResLoadCreate(): The sound tag '%s' references '%s' which is not included in the wav bank '%s'.\n",
										pSeq[i].pszName,
										pPlay->pszWavName,
										pHeader->pszBankName );
					// Nate - Don't fail the entire bank just because a single sound fails
					//return FALSE;
				}
				++uNumGoodSounds;
			}
		}
	}

	// If we don't have any valid sounds in the bank, fail it
	if( !uNumGoodSounds ) {
		return FALSE;	
	}
	
	// add this to our loaded bank list
	_apBanks[nBankIndex] = pHeader;
	_nNumLoadedBanks++;

	DEVPRINTF( "******** LOADED SFX BANK [%d] - '%s'***********.\n", _nNumLoadedBanks - 1, pszResName );

	fres_SetBase( hRes, pLoadedBase );

	return TRUE;
}

static void _ResLoadDestroy( void *pBase ) {
	FData_SFxBank_Header_t *pHeader = (FData_SFxBank_Header_t *)pBase;

	FASSERT( _bModuleInited );
	FASSERT( _nNumLoadedBanks );
	FASSERT( (u32)pHeader == (u32)_apBanks[_nNumLoadedBanks-1] );
	
	_nNumLoadedBanks--;
	_apBanks[_nNumLoadedBanks] = NULL;	
}

static FData_SFxBank_Seq_t *_ConvertHandleToSeqPtr( FSndFx_FxHandle_t hHandle ) {
	u32 nBankIndex, nFxIndex;

	if( hHandle == FSNDFX_INVALID_FX_HANDLE ) {
		// not a valid handle
		return NULL;
	}
	nBankIndex = _GET_BANK_ID_FROM_HANDLE( hHandle );
	nFxIndex = _GET_FX_INDEX_FROM_HANDLE( hHandle );

	if( nBankIndex >= _nNumLoadedBanks ) {
		return NULL;
	}
	if( nFxIndex >= _apBanks[nBankIndex]->nNumSFx ) {
		return NULL;
	}
	FData_SFxBank_Seq_t *pSeq = &_apBanks[nBankIndex]->paSeqs[nFxIndex];
	if( pSeq->hHandleToMyself != hHandle ) {
		return NULL;
	}
	return pSeq;
}

