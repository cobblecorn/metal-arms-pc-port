//////////////////////////////////////////////////////////////////////////////////////
// MsgBox.h - System for displaying a message box
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


#ifndef _MESSAGEBOX_H_
#define _MESSAGEBOX_H_

#include "fang.h"
#include "flinklist.h"
#include "fgamedata.h"
#include "ftex.h"
#include "gamepad.h"
#include "gameloop.h"


typedef void* MessageBox_Handle_t;

#define MESSAGEBOX_INVALID_HANDLE		( NULL )


FCLASS_ALIGN_PREFIX class CMsgBox {

// types
public:
	typedef enum {
		BUTTON_NONE			= 0x0,
		BUTTON_ACCEPT		= 0x1,
		BUTTON_CANCEL		= 0x2,
		BUTTON_ALTERNATE	= 0x4,
	} Button_e ;

protected:

private:
	typedef struct {
		CFVec2 		vPosUL;
		CFVec2 		vPosBR;

		FTexDef_t*	pTexDef;

		CFVec2 		vST0_Corner;
		CFVec2 		vST1_Corner;
		CFVec2 		vST0_Edge;
		CFVec2 		vST1_Edge;
		CFVec2 		vST0_Fill;
		CFVec2 		vST1_Fill;

		CFVec2 vTitlePos;

		
		CFVec2 vBodyPos;

		u32		uAButtonEnabled;
		CFVec2	vAButtonPos;

		u32		uBButtonEnabled;
		CFVec2	vBButtonPos;

		u32		uCButtonEnabled;
		CFVec2	vCButtonPos;

		cwchar *pwszTitleFormatStr;
		cwchar *pwszBodyFormatStr;
	} Gamedata_t;



// data
public:
protected:
private:
	static BOOL m_bSystemInitialized;
	static FLinkRoot_t m_LinkRoot;
	static BOOL m_bSystemActive;
	static const FGameData_TableEntry_t m_aGameDataVocab[];

	FLink_t		m_Link;
	BOOL		m_bShow;
	
	Gamedata_t	m_Data;
	BOOL		m_bDimScreen;

	cchar *m_pszName;
	f32	m_fX0;
	f32 m_fX1;
	f32	m_fY0;
	f32 m_fY1;
	u32 m_uMsgBoxLevel;
	u32 m_uLastContextData;

	BOOL m_bAButtonEnabled;
	BOOL m_bBButtonEnabled;
	BOOL m_bCButtonEnabled;

	CFTexInst m_BoxTex;
		
	cwchar *m_pwszTitle;
	cwchar *m_pwszBody;
	cwchar *m_pwszAButton;
	cwchar *m_pwszBButton;
	cwchar *m_pwszCButton;

	
	GameloopFcn_t	*m_pPrevFnWork;

	static CFTexInst		m_ButtonTex;
	static GamepadMap_e		m_PrevGamepadMap;
	static Button_e			m_eButton;

	static GameloopFcn_t	*m_pFnPrevWork;
	static GameloopFcn_t	*m_pFnPrevDraw;
	static GameloopFcn_t	*m_pFnPrevInit;
	static GameloopFcn_t	*m_pFnWork;

	static u32	m_uContextData;
	static BOOL m_bPausedGame;
	
	static BOOL m_bPausedGameLoop;
	static u32	m_uAudioPauseLevel;
	static u32	m_uStreamPauseLevel;
	static u32	m_uCurrentMsgBoxLevel;



// Functions
public:

	static BOOL InitSystem( void );
	static void UninitSystem( void );
    	
	// show or hide the message box
	static void Display( cchar *pszName, cwchar *pwszTitle, cwchar *pwszBody, cwchar *pwszAButton, cwchar *pwszBButton, cwchar *pwszCButton, u32 uData, BOOL bDimScreen=FALSE, GameloopFcn_t *pWorkFn=NULL, BOOL bPauseGame=TRUE );

	// removes the current message box
	static void Clear( void );

	static BOOL IsInitialized( void ) { return m_bSystemInitialized; }
	static BOOL IsActive( void ) { return m_bSystemActive; }

#if defined(MA_PC_INPUT)
	// PC: one line drawn under the current box (for example a key a box's retail data has no button
	// for). Display() clears it; the caller keeps the string alive.
	static void SetPcFooter( cwchar *pwszFooter ) { m_pwszPcFooter = pwszFooter; }
	static cwchar *m_pwszPcFooter;
#endif


	// see whether a button was pressed
	static Button_e CheckForButtonPress( BOOL bClearOnPress=FALSE )	{ return m_eButton; }
	static u32	GetContextData( void )		 						{ return m_uContextData; }
	static void ClearButtonPress( void )							{ m_eButton = BUTTON_NONE; }

	// init system from CSV data
	static BOOL InitFromCSV( void );

	static MessageBox_Handle_t GetMsgBoxByName( cchar *pszName );

	// this will reset the message box to default values, in case anything was overriden
	static void Reset( MessageBox_Handle_t hMsgBox );

	static void WorkAll( void );
	static void DrawAll( void );

	static BOOL GameWork( void );


	

protected:
private:
	static CMsgBox* _GetMsgBoxFromHandle( MessageBox_Handle_t hMsgBox );
	static CMsgBox* _CreateFromGameTable( FGameDataTableHandle_t hTable );

	CMsgBox();
	~CMsgBox();

	void _Draw( void );
	void _Work( void );
	void _DrawButtons( void );
	void _DimScreen( void );

	static void _PauseGame( void );
	static void _UnpauseGame( void );

	static BOOL _CheckForAButton( void );
	static BOOL _CheckForBButton( void );
	static BOOL _CheckForCButton( void );

FCLASS_STACKMEM_ALIGN( CMsgBox );
} FCLASS_ALIGN_SUFFIX;

#endif