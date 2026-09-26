//////////////////////////////////////////////////////////////////////////////////////
// fsndfx.h - 
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
// 06/16/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FSNDFX_H_
#define _FSNDFX_H_ 1

#include "fang.h"
#include "faudio.h"
#include "fdata.h"


#define FSNDFX_INVALID_BANK_HANDLE		( (FSndFx_BankHandle_t)0 )
#define FSNDFX_INVALID_FX_HANDLE		( (FSndFx_FxHandle_t)0 )




extern BOOL fsndfx_ModuleStartup( void );
extern void fsndfx_ModuleShutdown( void );

extern u32 fsndfx_GetNumLoadedBanks( void );
extern FSndFx_BankHandle_t fsndfx_GetBankHandle( u32 nBankIndex );

extern u32 fsndfx_GetNumFxsInBank( FSndFx_BankHandle_t hBank );
extern FSndFx_FxHandle_t fsndfx_GetFxHandle( FSndFx_BankHandle_t hBank, u32 nFxIndex );
extern FSndFx_FxHandle_t fsndfx_GetFxHandle( cchar *pszName );

extern cchar *fsndfx_ConvertFxHandleToName( FSndFx_FxHandle_t hHandle );
extern FAudio_WaveHandle_t fsndfx_GetWavHandle( FSndFx_FxHandle_t hHandle );
extern f32 fsndfx_GetUnitPlayVolume( FSndFx_FxHandle_t hHandle );
extern s32 fsndfx_GetNumLoops( FSndFx_FxHandle_t hHandle );
extern f32 fsndfx_GetDuration( FSndFx_FxHandle_t hHandle );

////////////////////////////////////////////////////////
// THESE FUNCTIONS ARE FIRE AND FORGET, NO EXTRA CLEANUP
extern BOOL fsndfx_Play2D( FSndFx_FxHandle_t hHandle,
						   f32 fVolumeMultiplier=1.0f,
						   f32 fFreqMultiplier=1.0f,
						   u8 nPriority=FAudio_EmitterDefaultPriorityLevel,
						   f32 fPanLeftRight=0.0f,
						   BOOL bDuckable=TRUE,
						   CFAudioEmitter **ppUserAudioEmitter=NULL);
extern BOOL fsndfx_Play3D( FSndFx_FxHandle_t hHandle,
						   const CFVec3A *pPos_WS,
						   f32 fOuterRadius, 
						   f32 fSpawnRadiusFactor=-1.0f,
						   f32 fVolumeMultiplier=1.0f,
						   f32 fFreqMultiplier=1.0f,
						   u8 nPriority=FAudio_EmitterDefaultPriorityLevel,
						   BOOL bDuckable=TRUE,
						   CFAudioEmitter **ppUserAudioEmitter=NULL);
							   
///////////////////////////////////////////////////////////////////////////////////////////
// THESE FUNCTIONS RETURN AN EMITTER PTR THAT THE CALLER MUST CALL DESTROY ON AT SOME POINT
#if !FANG_PRODUCTION_BUILD
	extern CFAudioEmitter *fsndfx_AllocNPlay2DSound( cchar *pszFileName, u32 nLineNum,
												FSndFx_FxHandle_t hHandle,
												f32 fVolumeMultiplier=1.0f,
												f32 fFreqMultiplier=1.0f,
												u8 nPriority=FAudio_EmitterDefaultPriorityLevel,
												f32 fPanLeftRight=0.0f,
												BOOL bDuckable=TRUE );
	extern CFAudioEmitter *fsndfx_AllocNPlay3DSound( cchar *pszFileName, u32 nLineNum,
												FSndFx_FxHandle_t hHandle,
												const CFVec3A *pPos_WS,
												f32 fOuterRadius, 
												f32 fVolumeMultiplier=1.0f,
												f32 fFreqMultiplier=1.0f,
												u8 nPriority=FAudio_EmitterDefaultPriorityLevel,
												BOOL bDuckable=TRUE );
	#define FSNDFX_ALLOCNPLAY2D( hHandle, fVolumeMultiplier, fFreqMultiplier, nPriority, fPanLeftRight, bDuckable ) \
				fsndfx_AllocNPlay2DSound( __FILE__, __LINE__, hHandle, fVolumeMultiplier, fFreqMultiplier, nPriority, fPanLeftRight, bDuckable )
	#define FSNDFX_ALLOCNPLAY3D( hHandle, pPos_WS, fOuterRadius, fVolumeMultiplier, fFreqMultiplier, nPriority, bDuckable ) \
				fsndfx_AllocNPlay3DSound( __FILE__, __LINE__, hHandle, pPos_WS, fOuterRadius, fVolumeMultiplier, fFreqMultiplier, nPriority, bDuckable )
#else
	extern CFAudioEmitter *fsndfx_AllocNPlay2DSound( FSndFx_FxHandle_t hHandle,
												f32 fVolumeMultiplier=1.0f,
												f32 fFreqMultiplier=1.0f,
												u8 nPriority=FAudio_EmitterDefaultPriorityLevel,
												f32 fPanLeftRight=0.0f,
												BOOL bDuckable=TRUE );
	extern CFAudioEmitter *fsndfx_AllocNPlay3DSound( FSndFx_FxHandle_t hHandle,
												const CFVec3A *pPos_WS,
												f32 fOuterRadius, 
												f32 fVolumeMultiplier=1.0f,
												f32 fFreqMultiplier=1.0f,
												u8 nPriority=FAudio_EmitterDefaultPriorityLevel,
												BOOL bDuckable=TRUE );
	#define FSNDFX_ALLOCNPLAY2D( hHandle, fVolumeMultiplier, fFreqMultiplier, nPriority, fPanLeftRight, bDuckable ) \
				fsndfx_AllocNPlay2DSound( hHandle, fVolumeMultiplier, fFreqMultiplier, nPriority, fPanLeftRight, bDuckable )
	#define FSNDFX_ALLOCNPLAY3D( hHandle, pPos_WS, fOuterRadius, fVolumeMultiplier, fFreqMultiplier, nPriority, bDuckable ) \
				fsndfx_AllocNPlay3DSound( hHandle, pPos_WS, fOuterRadius, fVolumeMultiplier, fFreqMultiplier, nPriority, bDuckable )
#endif

#endif

