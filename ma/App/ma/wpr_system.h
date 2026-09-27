//////////////////////////////////////////////////////////////////////////////////////
// wpr_system.h - the main wrapper system interface
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
// 11/05/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WPR_SYSTEM_H_
#define _WPR_SYSTEM_H_ 1

#include "fang.h"
#include "gamesave.h"
#include "game.h"
#include "wpr_datatypes.h"

#define WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN				-1

#define WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES		FALSE
#define WPR_SYSTEM_ALLOW_AUTO_CENTER_CHANGES			FALSE

extern BOOL wpr_system_InitSystem( void );
extern void wpr_system_UninitSystem( void );
extern BOOL wpr_system_AreWrappersReadyToUse( void );

extern BOOL wpr_system_InitInGame( void );

extern void wpr_system_Start( BOOL bBootup );
extern void wpr_system_End( void );

extern const GameInitInfo_t *wpr_system_GetInitInfo( void );

extern void wpr_system_ResetToStartupScreen( BOOL bBootup );

// subsystems used by other menu systems (such as the e3 menu)
typedef struct {
	u16 nCCNumScreenPosInfos;
    u16 nCCNumConfigs;	
	Wpr_DataTypes_ControllerTextPos_t *paCCScreenPosInfos;// nCCNumScreenPosInfos of these
	Wpr_DataTypes_ControllerConfig_t *paCCConfigs;// nCCNumConfigs of these
} WprSystem_ControllerConfigData_t;

extern s32 wpr_system_FindActiveControllerPort( BOOL bTestAcceptButton, BOOL bTestUDStick, BOOL bTestLRStick );
extern void wpr_system_DrawBasicScreen( Wpr_DataTypes_ScreenData_t *pScreen, s32 nSelectedIndex,
									   BOOL bDrawUnselectedAsThis, f32 fScaleMultiplier,
									   f32 fHalfXRes, f32 fHalfYRes );
extern const GameInitInfo_t *wpr_system_CreateSimpleSinglePlayerGameInit( BOOL bInvertControls );
extern const GameInitInfo_t *wpr_system_CreateSimpleMultiPlayerGameInit( u32 nNumPlayers, u32 nPlayerMask, u32 nLevelToPlay );

extern BOOL wpr_system_InitControllerConfigData( FGameDataFileHandle_t hFile, WprSystem_ControllerConfigData_t *pData,
												CFStringTable &rStringTable );
extern void wpr_system_ControllerConfig_DrawFDraw( Wpr_DataTypes_ControllerConfig_t *pConfig,
												   CFTexInst *pTexInst,
												   BOOL bDrawArrows,
											   f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes,
											   u32 nControllerPort );
extern void wpr_system_ControllerConfig_DrawOrtho( Wpr_DataTypes_ScreenData_t *pScreen, Wpr_DataTypes_ControllerConfig_t *pConfig,
												  cwchar *pwszConfigString, u32 nCurrentConfigIndex,
												  f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );

extern void wpr_system_IG_SelectScreen( Wpr_DataTypes_Screens_e nScreenIndex );
extern BOOL wpr_system_IG_Work( void );
extern void wpr_system_IG_Draw( void );

// ig save game.  Players may not exist, so it takes pointers to the profiles.
extern BOOL wpr_system_IG_SaveGame( CPlayerProfile *pProfile0,
								   CPlayerProfile *pProfile1 = NULL,
								   CPlayerProfile *pProfile2 = NULL,
								   CPlayerProfile *pProfile3 = NULL );
								   
extern BOOL wpr_system_CreateLoadHeading( u8 nLevelNum,
										 wchar *pwszBuffer, u32 nBufSize, 
										 wchar *pwszLocBuf=NULL, u32 nLocBufSize=0,
										 wchar *pwszTagBuf=NULL, u32 nTagBufSize=0 );

#endif
