//////////////////////////////////////////////////////////////////////////////////////
// MsgBox.cpp - System for displaying a message box
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
// 04.28.03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "msgbox.h"
#include "fgamedata.h"
#include "fres.h"
#include "fstringtable.h"
#include "fclib.h"
#include "fresload.h"
#include "gamepad.h"
#include "fcolor.h"
#include "fviewport.h"
#include "fxfm.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fvtxpool.h"
#include "floop.h"
#include "ftext.h"
#include "pausescreen.h"
#include "faudio.h"
#include "game.h"
#include "sas_user.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#include "wpr_drawutils.h"
#endif

#define _MSGBOX_FNAME		( "msgbox" )
#define _BUTTONWIDTH		( 0.05f )
#define _BUTTONHEIGHT		( 0.077f )
#define _BUTTONSPACE		( 0.000f )
#define _BUTTON_TEXT_STYLE	( L"~f8~w0~aL~C99999999~s0.80~b0%ls" )
#define _BUTTON_Y_ADJ		( 0.01f )
#if defined(MA_PC_INPUT)
// PC ftext retains the retail 0..0.75 vertical range at every display aspect.
#define _MSGBOX_TEXT_Y_SCALE ( 0.75f )
#else
#define _MSGBOX_TEXT_Y_SCALE ( FViewport_pDefaultOrtho->fAspectHOW )
#endif


BOOL				CMsgBox::m_bSystemInitialized	= FALSE;
FLinkRoot_t			CMsgBox::m_LinkRoot;
BOOL				CMsgBox::m_bSystemActive		= FALSE;
CFTexInst			CMsgBox::m_ButtonTex;
GamepadMap_e		CMsgBox::m_PrevGamepadMap;
GameloopFcn_t*		CMsgBox::m_pFnPrevWork			= NULL;
GameloopFcn_t*		CMsgBox::m_pFnPrevDraw			= NULL;
GameloopFcn_t*		CMsgBox::m_pFnPrevInit			= NULL;
GameloopFcn_t*		CMsgBox::m_pFnWork				= NULL;
#if defined(MA_PC_INPUT)
cwchar*				CMsgBox::m_pwszPcFooter			= NULL;
#endif
//GameloopFcn_t*		CMsgBox::m_pFnWork;
CMsgBox::Button_e	CMsgBox::m_eButton				= CMsgBox::BUTTON_NONE;
u32					CMsgBox::m_uContextData			= 0;
BOOL				CMsgBox::m_bPausedGame			= FALSE;
BOOL				CMsgBox::m_bPausedGameLoop		= FALSE;
u32					CMsgBox::m_uAudioPauseLevel		= 0;
u32					CMsgBox::m_uStreamPauseLevel	= 0;
u32					CMsgBox::m_uCurrentMsgBoxLevel	= 0;

BOOL CMsgBox::InitSystem( void ) {
	flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CMsgBox, m_Link ) );

	m_bSystemInitialized = TRUE;

	FTexDef_t *pTexDef = (FTexDef_t*)fresload_Load( FTEX_RESNAME, CPauseScreen_pszControlsTex );
	if( !pTexDef ) {
		DEVPRINTF( "CMsgBox::InitSystem():  Unable to load button texture\n" );
		return FALSE;
	}

	m_ButtonTex.SetTexDef( pTexDef );
	m_ButtonTex.ClearFlag( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T | CFTexInst::FLAG_WRAP_U );

	if( !InitFromCSV() ) {
		return FALSE;
	}

	return TRUE;
}

void CMsgBox::UninitSystem( void ) {
	m_bSystemInitialized = FALSE;

	CMsgBox *pMsgBox = (CMsgBox*)flinklist_RemoveHead( &m_LinkRoot );
	while( pMsgBox ) {
		fdelete( pMsgBox );
		pMsgBox = (CMsgBox*)flinklist_RemoveHead( &m_LinkRoot );
	}	
}


void CMsgBox::Display( cchar *pszName, cwchar *pwszTitle, cwchar *pwszBody, cwchar *pwszAButton, cwchar *pwszBButton, cwchar *pwszCButton, u32 uContextData, BOOL bDimScreen/*=FALSE*/, GameloopFcn_t *pWorkFn/*=NULL*/, BOOL bPauseGame/*=TRUE*/ ) {
	CMsgBox *pMsgBox = (CMsgBox*)GetMsgBoxByName( pszName );
	if( !pMsgBox ) {
		FASSERT_NOW;
		return;
	}

	Reset( (MessageBox_Handle_t)pMsgBox );

	m_uContextData = uContextData;
	m_bSystemActive = TRUE;
	pMsgBox->m_bShow = TRUE;
#if defined(MA_PC_INPUT)
	m_pwszPcFooter = NULL;
#endif

	m_uCurrentMsgBoxLevel++;
	pMsgBox->m_uMsgBoxLevel = m_uCurrentMsgBoxLevel;
	
	pMsgBox->m_pwszTitle = pwszTitle;
	pMsgBox->m_pwszBody	= pwszBody;
	pMsgBox->m_pwszAButton = pwszAButton;
	pMsgBox->m_pwszBButton = pwszBButton;
	pMsgBox->m_pwszCButton = pwszCButton;
	pMsgBox->m_bDimScreen = bDimScreen;
	pMsgBox->m_bAButtonEnabled = pMsgBox->m_Data.uAButtonEnabled && pwszAButton;
	pMsgBox->m_bBButtonEnabled = pMsgBox->m_Data.uBButtonEnabled && pwszBButton;
	pMsgBox->m_bCButtonEnabled = pMsgBox->m_Data.uCButtonEnabled && pwszCButton;

	if( m_uCurrentMsgBoxLevel == 1 ) {
		if( bPauseGame ) {
			m_bPausedGame = TRUE;
			_PauseGame();
		} else {
			m_bPausedGame = FALSE;
		}
	}

	pMsgBox->m_uLastContextData = m_uContextData;
	m_uContextData = uContextData;

	pMsgBox->m_pPrevFnWork = m_pFnWork;
	m_pFnWork = pWorkFn;

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "CMsgBox::Display():  MB = %s:  pWork = %x, m_pFnWork = %x, pMsgBox->m_pPrevFnWork = %x\n", pMsgBox->m_pszName, pWorkFn, m_pFnWork, pMsgBox->m_pPrevFnWork );
#endif
	
	m_eButton = BUTTON_NONE;
}


void CMsgBox::Clear( void ) {
	FASSERT( m_uCurrentMsgBoxLevel > 0 );

	CMsgBox *pMsgBox;

	for( pMsgBox = (CMsgBox*)flinklist_GetHead( &m_LinkRoot ); pMsgBox; pMsgBox = (CMsgBox*)flinklist_GetNext( &m_LinkRoot, pMsgBox ) ) {
		if( pMsgBox->m_uMsgBoxLevel == m_uCurrentMsgBoxLevel ) {
			pMsgBox->m_bShow = FALSE;
			m_pFnWork = pMsgBox->m_pPrevFnWork;
			pMsgBox->m_pPrevFnWork = NULL;
			pMsgBox->m_uMsgBoxLevel = (u32)-1;

			m_uContextData = pMsgBox->m_uLastContextData;
			pMsgBox->m_uLastContextData = 0;
			
			#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
				DEVPRINTF( "CMsgBox::Clear():  MB = %s:  pWork = %x, m_pFnWork = %x, pMsgBox->m_pPrevFnWork = %x\n", pMsgBox->m_pszName, NULL, m_pFnWork, pMsgBox->m_pPrevFnWork );
			#endif
		}
	}

	m_uCurrentMsgBoxLevel--;

	if( m_uCurrentMsgBoxLevel == 0 ) {
		_UnpauseGame();
	}


}


#define _NUM_CONTROLLERS		( 4 )


BOOL CMsgBox::GameWork( void ) {
	GamepadMap_e aPrevMap[_NUM_CONTROLLERS];
	u32 i;

	for( i=0; i < _NUM_CONTROLLERS; i++ ) {
		aPrevMap[i] = gamepad_GetMapping( i );
		gamepad_SetMapping(i, GAMEPAD_MAP_MENU );
	}

	gamepad_Sample();

	WorkAll();
	
	game_HandleControllersBeingUnplugged();

	// restore previous mapping
	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
		gamepad_SetMapping( i, aPrevMap[i] );
	}

	return TRUE;
}

// init a messagebox from CSV data
BOOL CMsgBox::InitFromCSV( void ) {
	FGameDataFileHandle_t	hFile;
	FGameDataWalker_t		gdataWalker;
	FGameDataTableHandle_t	hTable;
	
	FMemFrame_t memFrame	= fmem_GetFrame();
	FResFrame_t	resFrame	= fres_GetFrame();
	BOOL		bDataLoaded	= FALSE;
	CMsgBox*	pMsgBox		= NULL;

	hFile = fgamedata_LoadFileToFMem( _MSGBOX_FNAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CMsgBox::InitFromCSV():  Error loading file %s\n", _MSGBOX_FNAME );
		goto _ExitWithError;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, gdataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( gdataWalker ) ) {
		pMsgBox = _CreateFromGameTable( hTable );
		if( pMsgBox != MESSAGEBOX_INVALID_HANDLE ) { 
			flinklist_AddTail( &m_LinkRoot, pMsgBox );
		} else {
			DEVPRINTF( "CMsgBox::InitFromCSV():  Error loading %s\n", fgamedata_GetTableName( hTable ) );
			goto _ExitWithError;
		}
	}

    fmem_ReleaseFrame( memFrame );
	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( memFrame );
	fres_ReleaseFrame( resFrame );
	return FALSE;
}


CMsgBox* CMsgBox::_CreateFromGameTable( FGameDataTableHandle_t hTable ) {
	FASSERT( m_bSystemInitialized );

	FGameData_VarType_e nVarType;
	CMsgBox *pMsgBox	= NULL;
	cwchar *pwstr;
	cchar *pszTableName = fgamedata_GetTableName( hTable );
	

	if( GetMsgBoxByName( pszTableName ) != MESSAGEBOX_INVALID_HANDLE ) {
		DEVPRINTF( "CMsgBox::_CreateFromGameTable():  ERROR, duplicate entry found: %s\n", pszTableName );
		return MESSAGEBOX_INVALID_HANDLE;
	}

	pMsgBox = fnew CMsgBox;
	if( !pMsgBox ) {
		DEVPRINTF( "CMsgBox::_CreateFromGameTable():  Unable to allocate memory for new messagebox\n" );
		return MESSAGEBOX_INVALID_HANDLE;
	}

	pMsgBox->m_pszName = CFStringTable::AddString( NULL, pszTableName );

	if( !fgamedata_GetTableData( hTable, m_aGameDataVocab, &pMsgBox->m_Data, sizeof( /*CMsgBox::*/Gamedata_t ) - (2 * sizeof( cwchar*)), 0 ) ) {
		goto _ErrorReadingData;
	}

	

	pwstr = (wchar*)fgamedata_GetPtrToFieldData( hTable, 30, nVarType );
	if( !pwstr || (nVarType != FGAMEDATA_VAR_TYPE_WIDESTRING ) ) {
		goto _ErrorReadingData;
	}
	pMsgBox->m_Data.pwszTitleFormatStr = CFStringTable::AddString( NULL, pwstr );

	pwstr = (wchar*)fgamedata_GetPtrToFieldData( hTable, 31, nVarType );
	if( !pwstr || (nVarType != FGAMEDATA_VAR_TYPE_WIDESTRING ) ) {
		goto _ErrorReadingData;
	}
	pMsgBox->m_Data.pwszBodyFormatStr = CFStringTable::AddString( NULL, pwstr );

	pMsgBox->m_BoxTex.SetTexDef( pMsgBox->m_Data.pTexDef );

	return pMsgBox;

_ErrorReadingData:
	DEVPRINTF( "CMsgBox::_CreateFromGameTable():  Error parsing table %s\n", pszTableName );
	fdelete( pMsgBox );
	return NULL;
}


void CMsgBox::WorkAll( void ) {
	if( !m_bSystemActive ) {
		return;
	}

	CMsgBox *pMsgBox;
	m_bSystemActive = FALSE;


	for( pMsgBox = (CMsgBox*)flinklist_GetHead( &m_LinkRoot ); pMsgBox; pMsgBox = (CMsgBox*)flinklist_GetNext( &m_LinkRoot, pMsgBox ) ) {
		if( pMsgBox->m_bShow && (pMsgBox->m_uMsgBoxLevel == m_uCurrentMsgBoxLevel) ) {
			m_bSystemActive = TRUE;
			break;
		}
	}

	if( pMsgBox ) {
		pMsgBox->_Work();
	}
}


void CMsgBox::DrawAll( void ) {
	if( !m_bSystemActive ) {
		return;
	}

	CMsgBox *pMsgBox;

	for( pMsgBox = (CMsgBox*)flinklist_GetHead( &m_LinkRoot ); pMsgBox; pMsgBox = (CMsgBox*)flinklist_GetNext( &m_LinkRoot, pMsgBox ) ) {
		if( pMsgBox->m_bShow ) {
			pMsgBox->_Draw();
		}
	}
}



CMsgBox* CMsgBox::_GetMsgBoxFromHandle( MessageBox_Handle_t hMsgBox ) {
	FASSERT( hMsgBox );

	return (CMsgBox*)hMsgBox;
}


MessageBox_Handle_t CMsgBox::GetMsgBoxByName( cchar *pszName ) {
	CMsgBox *pMsgBox;
	for( pMsgBox=(CMsgBox*)flinklist_GetHead( &m_LinkRoot ); pMsgBox; pMsgBox=(CMsgBox*)flinklist_GetNext( &m_LinkRoot, pMsgBox ) ) {
		if( !fclib_stricmp( pszName, pMsgBox->m_pszName ) ) {
			return (MessageBox_Handle_t)pMsgBox;
		}
	}
	return MESSAGEBOX_INVALID_HANDLE;
}


void CMsgBox::Reset( MessageBox_Handle_t hMsgBox ) {
	CMsgBox *pMsgBox = _GetMsgBoxFromHandle( hMsgBox );
	if( !pMsgBox ) {
		return;
	}

	pMsgBox->m_fX0 = pMsgBox->m_Data.vPosUL.x;
	pMsgBox->m_fX1 = pMsgBox->m_Data.vPosBR.x;
	pMsgBox->m_fY0 = pMsgBox->m_Data.vPosUL.y;
	pMsgBox->m_fY1 = pMsgBox->m_Data.vPosBR.y;
}



////////////////////////
//NON STATICS

CMsgBox::CMsgBox() {
	m_bShow				= FALSE;
	m_pPrevFnWork		= NULL;
	m_uLastContextData	= 0;
}


CMsgBox::~CMsgBox() {
}


void CMsgBox::_PauseGame( void ) {

	// pause the game loop
	m_bPausedGameLoop = !FLoop_bGamePaused;
	if( m_bPausedGameLoop ) {
        floop_PauseGame( TRUE );
	}

	// pause the audio
	m_uAudioPauseLevel = CFAudioEmitter::GetGlobalPauseLevel();
	if( m_uAudioPauseLevel < FAUDIO_PAUSE_LEVEL_5 ) {
	    CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_5 );
	}

	// stop any force feedback
	fforce_Reset();

	// hijack the work fn
	GameloopFcn_t *pCurrentWork;

	gameloop_GetLoopHandlers( &pCurrentWork, &m_pFnPrevDraw, &m_pFnPrevInit );
	if( pCurrentWork != CMsgBox::GameWork ) {
		m_pFnPrevWork = pCurrentWork;
	}

	gameloop_SetLoopHandlers( GameWork, m_pFnPrevDraw, NULL );
}


void CMsgBox::_UnpauseGame( void ) {

	if( !m_bPausedGame ) {
		return;
	}
	
	// if we are here, we paused paused the game, so lets unpause it
	if( m_bPausedGameLoop ) {
		floop_PauseGame( FALSE );
	}
	
    // put the audio back like it was
	CFAudioEmitter::SetGlobalPauseLevel( (FAudio_PauseLevel_e)m_uAudioPauseLevel );

	// free the hostage work fn
	//gameloop_SetLoopHandlers( m_pFnPrevWork, m_pFnPrevDraw, m_pFnPrevInit );
	gameloop_SetLoopHandlers( m_pFnPrevWork, m_pFnPrevDraw, NULL );

}


void CMsgBox::_Work( void ) {

	if( m_pFnWork ) {
		m_pFnWork();
	}	

	if( (m_eButton == BUTTON_NONE) && (m_bAButtonEnabled && _CheckForAButton()) ) {
		if( !m_pFnWork ) {
			Clear();
		}
	
		m_eButton = BUTTON_ACCEPT;
		return;
	}

	if( (m_eButton == BUTTON_NONE) && (m_bBButtonEnabled && _CheckForBButton()) ) {
		if( !m_pFnWork ) {
			Clear();
		}
		m_eButton = BUTTON_CANCEL;
		return;
	}

	if( (m_eButton == BUTTON_NONE) && (m_bCButtonEnabled && _CheckForCButton()) ) {
		if( !m_pFnWork ) {
			Clear();
		}
		m_eButton = BUTTON_ALTERNATE;
		return;
	}

}


BOOL CMsgBox::_CheckForAButton( void ) {
	for( u32 i=0; i<_NUM_CONTROLLERS; i++ ) {
		if( Gamepad_aapSample[i][GAMEPAD_MENU_START]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			return TRUE;
		}

		if( Gamepad_aapSample[i][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CMsgBox::_CheckForBButton( void ) {
	for( u32 i=0; i<_NUM_CONTROLLERS; i++ ) {
		if( Gamepad_aapSample[i][GAMEPAD_MENU_BACK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			return TRUE;
		}

		if( Gamepad_aapSample[i][GAMEPAD_MENU_BACK2]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CMsgBox::_CheckForCButton( void ) {
	for( u32 i=0; i<_NUM_CONTROLLERS; i++ ) {
		if( Gamepad_aapSample[i][GAMEPAD_MENU_BUTTON_TOP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			return TRUE;
		}
	}
	return FALSE;
}


void CMsgBox::_Draw( void ) {
	u32 i;
	FDrawVtx_t *pVtxPool = fvtxpool_GetArray( 28 );
	if( !pVtxPool ) {
		return;
	}

	FViewport_t *pPreviousVP = fviewport_GetActive();
	fviewport_SetActive( FViewport_pDefaultOrtho );
	FXfm_Identity.InitStackWithView();
	frenderer_Push( FRENDERER_DRAW, NULL );


	// dim the screen if necessary
	if( m_bDimScreen ) {
		_DimScreen();
	}



	// draw the corners
	f32 fCornerSizeX = 0.06f; //FMATH_FABS( m_Data.vST1_Corner.x - m_Data.vST0_Corner.x );
	f32 fCornerSizeY = fCornerSizeX * FViewport_pDefaultOrtho->fAspectWOH;


	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AT );
	fdraw_SetTexture( &m_BoxTex );

	//UL
	pVtxPool[0].Pos_MS.Set( m_fX0,					m_fY0,					1.0f );
	pVtxPool[1].Pos_MS.Set( m_fX0,					m_fY0 + fCornerSizeY,	1.0f );
	pVtxPool[2].Pos_MS.Set( m_fX0 + fCornerSizeX, 	m_fY0,					1.0f );
	pVtxPool[3].Pos_MS.Set( m_fX0 + fCornerSizeX, 	m_fY0 + fCornerSizeY,	1.0f );
	pVtxPool[4].Pos_MS.Set( m_fX1 - fCornerSizeX, 	m_fY0,					1.0f );
	pVtxPool[5].Pos_MS.Set( m_fX1 - fCornerSizeX, 	m_fY0 + fCornerSizeY,	1.0f );

	pVtxPool[6].Pos_MS.Set( m_fX1,					m_fY0,					1.0f );
	pVtxPool[7].Pos_MS.Set( m_fX1 - fCornerSizeX,	m_fY0,					1.0f );
	pVtxPool[8].Pos_MS.Set( m_fX1,				 	m_fY0 + fCornerSizeY,	1.0f );
	pVtxPool[9].Pos_MS.Set( m_fX1 - fCornerSizeX, 	m_fY0 + fCornerSizeY,	1.0f );
	pVtxPool[10].Pos_MS.Set( m_fX1,				 	m_fY1 - fCornerSizeY,	1.0f );
	pVtxPool[11].Pos_MS.Set( m_fX1 - fCornerSizeX, 	m_fY1 - fCornerSizeY,	1.0f );
	
	pVtxPool[12].Pos_MS.Set( m_fX1,					m_fY1,					1.0f );
	pVtxPool[13].Pos_MS.Set( m_fX1,					m_fY1 - fCornerSizeY,	1.0f );
	pVtxPool[14].Pos_MS.Set( m_fX1 - fCornerSizeX,	m_fY1,					1.0f );
	pVtxPool[15].Pos_MS.Set( m_fX1 - fCornerSizeX, 	m_fY1 - fCornerSizeY,	1.0f );
	pVtxPool[16].Pos_MS.Set( m_fX0 + fCornerSizeX,	m_fY1,					1.0f );
	pVtxPool[17].Pos_MS.Set( m_fX0 + fCornerSizeX, 	m_fY1 - fCornerSizeY,	1.0f );
	
	pVtxPool[18].Pos_MS.Set( m_fX0,					m_fY1,					1.0f );
	pVtxPool[19].Pos_MS.Set( m_fX0 + fCornerSizeX,	m_fY1,					1.0f );
	pVtxPool[20].Pos_MS.Set( m_fX0,				 	m_fY1 - fCornerSizeY,	1.0f );
	pVtxPool[21].Pos_MS.Set( m_fX0 + fCornerSizeX, 	m_fY1 - fCornerSizeY,	1.0f );
	pVtxPool[22].Pos_MS.Set( m_fX0,				 	m_fY0 + fCornerSizeY,	1.0f );
	pVtxPool[23].Pos_MS.Set( m_fX0 + fCornerSizeX, 	m_fY0 + fCornerSizeY,	1.0f );

	pVtxPool[24].Pos_MS.Set( m_fX0 + fCornerSizeX, m_fY0 + fCornerSizeY, 1.0f );
	pVtxPool[24].ST.Set( m_Data.vST0_Fill.x, m_Data.vST0_Fill.y );

	pVtxPool[25].Pos_MS.Set( m_fX1 - fCornerSizeX, m_fY0 + fCornerSizeY, 1.0f );
	pVtxPool[25].ST.Set( m_Data.vST1_Fill.x, m_Data.vST0_Fill.y );

	pVtxPool[26].Pos_MS.Set( m_fX0 + fCornerSizeX, m_fY1 - fCornerSizeY, 1.0f );
	pVtxPool[26].ST.Set( m_Data.vST0_Fill.x, m_Data.vST0_Fill.y );

	pVtxPool[27].Pos_MS.Set( m_fX1 - fCornerSizeX, m_fY1 - fCornerSizeY, 1.0f );
	pVtxPool[27].ST.Set( m_Data.vST1_Fill.x, m_Data.vST1_Fill.y );

	for( i=0; i<4; i++ ) {
		pVtxPool[i*6].ST.Set( m_Data.vST0_Corner.x, m_Data.vST0_Corner.y );
		pVtxPool[i*6+1].ST.Set( m_Data.vST0_Corner.x, m_Data.vST1_Corner.y );
		pVtxPool[i*6+2].ST.Set( m_Data.vST1_Corner.x, m_Data.vST0_Corner.y );
		pVtxPool[i*6+3].ST.Set( m_Data.vST1_Corner.x, m_Data.vST1_Corner.y );
		pVtxPool[i*6+4].ST.Set( m_Data.vST1_Edge.x, m_Data.vST0_Edge.y );
		pVtxPool[i*6+5].ST.Set( m_Data.vST1_Edge.x, m_Data.vST1_Edge.y );
	}

	for( i=0; i<28; i++ ) {
		pVtxPool[i].Pos_MS.x *= FViewport_pDefaultOrtho->Res.x;
		pVtxPool[i].Pos_MS.y *= FViewport_pDefaultOrtho->Res.y;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool,		6 );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool+6,	6 );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool+12,	6 );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool+18,	6 );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool+24,	4 );


	// get rid of any other text, so you can read this text
	ftext_ClearAllPending();

	if( m_pwszTitle ) {
		ftext_Printf( m_Data.vTitlePos.x, m_Data.vTitlePos.y * _MSGBOX_TEXT_Y_SCALE, m_Data.pwszTitleFormatStr, m_pwszTitle );
	}
	
	if( m_pwszBody ) {
		ftext_Printf( m_Data.vBodyPos.x, m_Data.vBodyPos.y * _MSGBOX_TEXT_Y_SCALE, m_Data.pwszBodyFormatStr, m_pwszBody );
	}
#if defined(MA_PC_INPUT)
	if( m_pwszPcFooter ) {
		ftext_Printf( 0.5f, (m_Data.vPosBR.y + 0.03f) * _MSGBOX_TEXT_Y_SCALE, L"~f1~C88641799~w0~aC~s1.00%ls", m_pwszPcFooter );
	}
#endif

	_DrawButtons();


	fvtxpool_ReturnArray( pVtxPool );
	frenderer_Pop();
	fviewport_SetActive( pPreviousVP );
}


#if defined(MA_PC_INPUT)
// Replace the retail A/B/Y prompt art with the selected PC theme. The vertices have already been
// converted to viewport pixels by the caller; generated prompts draw in viewport space.
static BOOL _DrawPcMessageButton( FDrawVtx_t *pVertices, u32 nButton, CFTexInst *pButtonTexture ) {
	const f32 fCenterX = 0.5f * (pVertices[0].Pos_MS.x + pVertices[2].Pos_MS.x);
	f32 fCenterY = 0.5f * (pVertices[0].Pos_MS.y + pVertices[1].Pos_MS.y);
	const f32 fWidth = FMATH_FABS( pVertices[2].Pos_MS.x - pVertices[0].Pos_MS.x );
	const f32 fHeight = FMATH_FABS( pVertices[1].Pos_MS.y - pVertices[0].Pos_MS.y );
	f32 fPromptRight = fCenterX + 0.5f * fWidth;
	f32 fRadius = 0.40f * FMATH_MIN( fWidth, fHeight );
	f32 fLeft, fTop, fRight, fBottom;
	if( ftext_GetLastPrintBounds( &fLeft, &fTop, &fRight, &fBottom ) && fBottom > fTop ) {
		fCenterY = 0.5f * (fTop + fBottom) * FViewport_pDefaultOrtho->Res.y;
		fPromptRight = (fLeft - 0.008f) * FViewport_pDefaultOrtho->Res.x;
		fRadius = 0.5f * (fBottom - fTop) * FViewport_pDefaultOrtho->Res.y;
	}
	if( !pcinput_UseKeyboardPrompts() && !pcinput_UsePlayStationPrompts() ) {
		// Keep the retail Xbox art, with the same visible size/center as generated glyphs.
		// The atlas cell has padding; its visible button fills about 70 percent.
		const f32 fHalfArt = fRadius / 0.70f;
		const f32 fX = fPromptRight - fRadius;
		pVertices[0].Pos_MS.Set( fX - fHalfArt, fCenterY - fHalfArt, 1.0f );
		pVertices[1].Pos_MS.Set( fX - fHalfArt, fCenterY + fHalfArt, 1.0f );
		pVertices[2].Pos_MS.Set( fX + fHalfArt, fCenterY - fHalfArt, 1.0f );
		pVertices[3].Pos_MS.Set( fX + fHalfArt, fCenterY + fHalfArt, 1.0f );
		return FALSE;
	}
	if( pcinput_UsePlayStationPrompts() ) {
		wpr_drawutils_DrawPlayStationGlyph( nButton, fPromptRight - fRadius, fCenterY,
			fRadius, TRUE );
	} else {
		static cwchar *apwszKeyLabels[] = { L"Space", L"Esc", L"E" };
		// A word such as Space is wider than the retail A glyph. Anchor its right edge
		// before the measured action label and cap the scale so the two cannot overlap.
		wpr_drawutils_DrawKeyCapCentered( apwszKeyLabels[nButton], fPromptRight / FViewport_pDefaultOrtho->Res.x,
			fCenterY / FViewport_pDefaultOrtho->Res.y, L'R',
			FMATH_MIN( 0.60f, fHeight / ( FViewport_pDefaultOrtho->Res.y * 0.041f ) ), 0.0f,
			FViewport_pDefaultOrtho->HalfRes.x, FViewport_pDefaultOrtho->HalfRes.y, NULL, NULL, NULL, NULL, TRUE );
	}
	fdraw_SetTexture( pButtonTexture );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	return TRUE;
}
#endif

void CMsgBox::_DrawButtons( void ) {
	f32 fButtonSizeX, fButtonSizeY;
	u32 i;

	FDrawVtx_t *pVtxPool = fvtxpool_GetArray( 4 );
	if( !pVtxPool ) {
		return;
	}

	fdraw_SetTexture( &m_ButtonTex );

    // draw A button if required
	if( m_bAButtonEnabled ) {
		fButtonSizeX = _BUTTONWIDTH; //CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].x - CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].x;
		fButtonSizeY = _BUTTONWIDTH	* FViewport_pDefaultOrtho->fAspectWOH; // 		_BUTTONHEIGHT; //CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].y - CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].y;

		pVtxPool[0].Pos_MS.Set( m_Data.vAButtonPos.x,					m_Data.vAButtonPos.y,					1.0f );
		pVtxPool[1].Pos_MS.Set( m_Data.vAButtonPos.x,					m_Data.vAButtonPos.y + fButtonSizeY,	1.0f );
		pVtxPool[2].Pos_MS.Set( m_Data.vAButtonPos.x + fButtonSizeX,	m_Data.vAButtonPos.y,					1.0f );
		pVtxPool[3].Pos_MS.Set( m_Data.vAButtonPos.x + fButtonSizeX,	m_Data.vAButtonPos.y + fButtonSizeY,	1.0f );

		pVtxPool[0].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].y );
		pVtxPool[1].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].y );
		pVtxPool[2].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].y );
		pVtxPool[3].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].y );

		for( i=0; i<4; i++ ) {
			pVtxPool[i].Pos_MS.x *= FViewport_pDefaultOrtho->Res.x;
			pVtxPool[i].Pos_MS.y *= FViewport_pDefaultOrtho->Res.y;
		}

		// Measure the action label before drawing a PC prompt beside it.
		ftext_Printf( m_Data.vAButtonPos.x + fButtonSizeX + _BUTTONSPACE, m_Data.vAButtonPos.y * _MSGBOX_TEXT_Y_SCALE + _BUTTON_Y_ADJ, _BUTTON_TEXT_STYLE, m_pwszAButton );

#if defined(MA_PC_INPUT)
		if( !_DrawPcMessageButton( pVtxPool, 0, &m_ButtonTex ) )
#endif
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool, 4 );



	}

	// draw B button if required
	if( m_bBButtonEnabled ) {
		fButtonSizeX = _BUTTONWIDTH;
		fButtonSizeY = _BUTTONWIDTH	* FViewport_pDefaultOrtho->fAspectWOH;

		pVtxPool[0].Pos_MS.Set( m_Data.vBButtonPos.x,					m_Data.vBButtonPos.y,					1.0f );
		pVtxPool[1].Pos_MS.Set( m_Data.vBButtonPos.x,					m_Data.vBButtonPos.y + fButtonSizeY,	1.0f );
		pVtxPool[2].Pos_MS.Set( m_Data.vBButtonPos.x + fButtonSizeX,	m_Data.vBButtonPos.y,					1.0f );
		pVtxPool[3].Pos_MS.Set( m_Data.vBButtonPos.x + fButtonSizeX,	m_Data.vBButtonPos.y + fButtonSizeY,	1.0f );

		pVtxPool[0].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].y );
		pVtxPool[1].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].y );
		pVtxPool[2].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].y );
		pVtxPool[3].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].y );

		for( i=0; i<4; i++ ) {
			pVtxPool[i].Pos_MS.x *= FViewport_pDefaultOrtho->Res.x;
			pVtxPool[i].Pos_MS.y *= FViewport_pDefaultOrtho->Res.y;
		}

		// Measure the action label before drawing a PC prompt beside it.
		ftext_Printf( m_Data.vBButtonPos.x + fButtonSizeX + _BUTTONSPACE, m_Data.vBButtonPos.y * _MSGBOX_TEXT_Y_SCALE + _BUTTON_Y_ADJ, _BUTTON_TEXT_STYLE, m_pwszBButton );

#if defined(MA_PC_INPUT)
		if( !_DrawPcMessageButton( pVtxPool, 1, &m_ButtonTex ) )
#endif
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool, 4 );



	}

	// draw C (alternate) button if required
	if( m_bCButtonEnabled ) {
		fButtonSizeX = _BUTTONWIDTH;
		fButtonSizeY = _BUTTONWIDTH	* FViewport_pDefaultOrtho->fAspectWOH;

		pVtxPool[0].Pos_MS.Set( m_Data.vCButtonPos.x,					m_Data.vCButtonPos.y,					1.0f );
		pVtxPool[1].Pos_MS.Set( m_Data.vCButtonPos.x,					m_Data.vCButtonPos.y + fButtonSizeY,	1.0f );
		pVtxPool[2].Pos_MS.Set( m_Data.vCButtonPos.x + fButtonSizeX,	m_Data.vCButtonPos.y,					1.0f );
		pVtxPool[3].Pos_MS.Set( m_Data.vCButtonPos.x + fButtonSizeX,	m_Data.vCButtonPos.y + fButtonSizeY,	1.0f );

		pVtxPool[0].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_Y].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_Y].y );
		pVtxPool[1].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_Y].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_Y].y );
		pVtxPool[2].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_Y].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_Y].y );
		pVtxPool[3].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_Y].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_Y].y );

		for( i=0; i<4; i++ ) {
			pVtxPool[i].Pos_MS.x *= FViewport_pDefaultOrtho->Res.x;
			pVtxPool[i].Pos_MS.y *= FViewport_pDefaultOrtho->Res.y;
		}

		// Measure the action label before drawing a PC prompt beside it.
		ftext_Printf( m_Data.vCButtonPos.x + fButtonSizeX + _BUTTONSPACE, m_Data.vCButtonPos.y * _MSGBOX_TEXT_Y_SCALE + _BUTTON_Y_ADJ, _BUTTON_TEXT_STYLE, m_pwszCButton );

#if defined(MA_PC_INPUT)
		if( !_DrawPcMessageButton( pVtxPool, 2, &m_ButtonTex ) )
#endif
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxPool, 4 );



	}


	fvtxpool_ReturnArray( pVtxPool );

}


void CMsgBox::_DimScreen( void ) {
	// renderer should be set up
	CFColorRGBA color;
	CFVec3 vtx0, vtx1, vtx2, vtx3;
	color.Set( 0.0f, 0.20f );

	vtx0.Set( 0.0f, FViewport_pDefaultOrtho->Res.y, 1.0f );
	vtx1.Set( 0.0f, 0.0f, 1.0f );
	vtx2.Set( FViewport_pDefaultOrtho->Res.x, 0.0f, 1.0f );
	vtx3.Set( FViewport_pDefaultOrtho->Res.x, FViewport_pDefaultOrtho->Res.y, 1.0f );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_DST );
	fdraw_SetTexture( NULL );

	fdraw_SolidQuad( &vtx0, &vtx1, &vtx2, &vtx3, &color );
}
















////////////////////

const FGameData_TableEntry_t CMsgBox::m_aGameDataVocab[] = {

	// CFVec2 vPosUL;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	
	// CFVec2 vPosBR;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	
	//FTexDef_t*	pTexDef;
	FGAMEDATA_VOCAB_TEXDEF,

	// CFVec2 m_vST0_Corner;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	
	// CFVec2 m_vST1_Corner;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// CFVec2 		m_vST0_Edge;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// CFVec2 		m_vST1_Edge;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// CFVec2 		m_vST0_Fill;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// CFVec2 		m_vST1_Fill;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	
	// CFVec2 m_vTitlePos;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// CFVec2 m_vBodyPos;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// u32 uAButtonEnabled
	FGAMEDATA_VOCAB_U32_BOUND(F32_DATATABLE_0,F32_DATATABLE_1),

	// CFVec2 m_vAButtonPos;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// u32 uBButtonEnabled
	FGAMEDATA_VOCAB_U32_BOUND(F32_DATATABLE_0,F32_DATATABLE_1),

	// CFVec2 m_vBButtonPos;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// u32 uCButtonEnabled
	FGAMEDATA_VOCAB_U32_BOUND(F32_DATATABLE_0,F32_DATATABLE_1),

	// CFVec2 m_vCButtonPos;
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0

};



















