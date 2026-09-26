//////////////////////////////////////////////////////////////////////////////////////
// viewer.cpp - Art Viewer.
//
// Author: Albert Yale
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
// 03/12/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "viewer.h"
#include "fresload.h"
#include "ftext.h"
#include "ffile.h"
#include "floop.h"
#include "fpadio.h"
#include "fpad.h"
#include "fstorage.h"
#include "fmath.h"
#include "ftimer.h"
#include "fdraw.h"
#include "fdata.h"
#include "fanim.h"
#include "fworld.h"
#include "fsysinfo.h"
#include "fversion.h"
#include "faudio.h"
#include "fperf.h"
#include "fvis.h"
#include "fclib.h"
#include "frenderer.h"

#include <string.h>

//ARG - >>>>>
#include "fDataStreaming.h"

#if FANG_PLATFORM_PS2
#include "ps2\fps2tex.h"
#endif
//ARG - <<<<<

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _AUDIO_TEST							FALSE

#define _SAFE_FMEM_RELEASEFRAME( p )		{ if( p ){ fmem_ReleaseFrame( p ); p = NULL; } }
#define _SAFE_FRES_RELEASEFRAME( p )		{ if( p ){ fres_ReleaseFrame( p ); p = NULL; } }

#define _SAFE_FDELETE( p )					{ if( p ){ fdelete( p ); p = NULL; } }
#define _SAFE_FDELETE_ARRAY( p )			{ if( p ){ fdelete_array( p ); p = NULL; } }

#define _PROFILE_NAME						L"Viewer v.1.0"
#define _APP_TITLE							"Viewer"

#define _MENU_LINES_PER_COLS				( 20 )
#define _MENU_COLS_PER_WINDOW				( 3 )

#define _MOVEMENT_SPEED_SUBJECT_ROTATION	( FMATH_DEG2RAD( 1.60f * 100.0f ) )
#define _MOVEMENT_SPEED_SLIDER				( 0.01f )

#define _MAX_XLAT_SPEED						( 0.85f * 95.0f )
#define _MAX_ZOOM_SPEED						( 0.65f * 115.0f )

#define _NEARPLANE							( 0.1f )
#define _FARPLANE							( 5000.0f )

#define _FILTER_NUM_CHARS_IN_SET			( 38 )
#define _FILTER_ANY_CHAR					'?'

#define _DEFAULT_BACKGROUND_COLOR_R			( 0.0f )
#define _DEFAULT_BACKGROUND_COLOR_G			( 0.0f )
#define _DEFAULT_BACKGROUND_COLOR_B			( 0.0f )
#define _DEFAULT_BACKGROUND_COLOR_I			( 0.0f )

#define _DEFAULT_AMBIENT_LIGHT_R			( 0.1f )
#define _DEFAULT_AMBIENT_LIGHT_G			( 0.1f )
#define _DEFAULT_AMBIENT_LIGHT_B			( 0.1f )
#define _DEFAULT_AMBIENT_LIGHT_I			( 0.1f )

#define _DEFAULT_DIRECTIONAL_LIGHT_R		( 1.0f )
#define _DEFAULT_DIRECTIONAL_LIGHT_G		( 1.0f )
#define _DEFAULT_DIRECTIONAL_LIGHT_B		( 1.0f )
#define _DEFAULT_DIRECTIONAL_LIGHT_I		( 1.0f )

#define _DEFAULT_DIRECTIONAL_LIGHT_X		( + 1.0f )
#define _DEFAULT_DIRECTIONAL_LIGHT_Y		( - 1.0f )
#define _DEFAULT_DIRECTIONAL_LIGHT_Z		( + 1.0f )

/*
#define _DIRECTIONAL_LIGHT_ARROW_X			( -15.0f )
#define _DIRECTIONAL_LIGHT_ARROW_Y			( 10.0f )
#define _DIRECTIONAL_LIGHT_ARROW_Z			( 25.0f )
*/
#define _DIRECTIONAL_LIGHT_ARROW_X			( 0.0f )
#define _DIRECTIONAL_LIGHT_ARROW_Y			( 0.0f )
#define _DIRECTIONAL_LIGHT_ARROW_Z			( 0.0f )
/*
#define _DIRECTIONAL_LIGHT_ARROW_X			( 0.0f )
#define _DIRECTIONAL_LIGHT_ARROW_Y			( 0.0f )
#define _DIRECTIONAL_LIGHT_ARROW_Z			( 8.0f )
*/
#define _DIRECTIONAL_LIGHT_ARROW_R			( 175.0f / 255.0f )
#define _DIRECTIONAL_LIGHT_ARROW_G			( 180.0f / 255.0f )
#define _DIRECTIONAL_LIGHT_ARROW_B			( 185.0f / 255.0f )
#define _DIRECTIONAL_LIGHT_ARROW_A			( 0.65f )
//#define _DIRECTIONAL_LIGHT_ARROW_A		( 1.0f )

#define _DIRECTIONAL_LIGHT_ARROW_OUTLINE_R	( 39.0f / 99.0f )
#define _DIRECTIONAL_LIGHT_ARROW_OUTLINE_G	( 39.0f / 99.0f )
#define _DIRECTIONAL_LIGHT_ARROW_OUTLINE_B	( 0.0f )
#define _DIRECTIONAL_LIGHT_ARROW_OUTLINE_A	( 1.0f )

#define _MAIN_MENU_NUM_ENTIRES				( 13 )
#define _COLOR_MENU_NUM_ENTIRES				( 4 )
#define _RESOURCE_NAME_LENGTH				( 11 )

#define _MENU_BACKGROUND_ALPHA				( 0.75f )

#define _MENU_COLOR_SAMPLE_BOX_TOP			( 0.18f )
#define _MENU_COLOR_SAMPLE_BOX_BOTTOM		( 0.22f )
#define _MENU_COLOR_SAMPLE_BOX_LEFT			( 0.32f )
#define _MENU_COLOR_SAMPLE_BOX_RIGHT		( 0.72f )

#define _CAMERA_PITCH_MIN					( - ( FMATH_HALF_PI - FMATH_DEG2RAD( 0.5f ) ) )
#define _CAMERA_PITCH_MAX					( FMATH_HALF_PI - FMATH_DEG2RAD( 0.5f ) )

#define _PAD_ERROR_MESSAGE_DELAY			( 0.5f )

#if _AUDIO_TEST
	#define _USAGE_STATS					TRUE
	#define _USAGE_REPORT_DELAY				( 60.0f )

	#define _AUDIO_POSITION_X				( 0.0f )
	#define _AUDIO_POSITION_Y				( 5.0f )
	#define _AUDIO_POSITION_Z				( -150.0f )
	//#define _AUDIO_POSITION_Z				( 30.0f )

	#define _AUDIO_RADIUS_OUTER				( 200.0f )
	//#define _AUDIO_RADIUS_OUTER			( 20.0f )
#endif

//ARG - >>>>>
#if FANG_PLATFORM_PS2
#define	_SCALE_TEXTURE						0.025f
#endif
//ARG - <<<<<

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum
{
	_MOVEMENT_TYPE_MESH_MOVE				= 0,
	_MOVEMENT_TYPE_DIR_LIGHT_ORIENTATION,
	_MOVEMENT_TYPE_CAMERA_MOVE,

	_MOVEMENT_TYPE_COUNT

} _MovementTypeID_e;

typedef enum
{
	_RESOURCE_TYPE_WORLD		= 1,
	_RESOURCE_TYPE_MESH,
	_RESOURCE_TYPE_TEXTURE,

} _ResourceTypeID_e;

typedef enum
{
	_MENU_MAINMENU				= 0x00000001,
	_MENU_MAINMENU_MASK			= 0x000000ff,

	_MENU_SELECT_WORLD			= 0x00000100,
	_MENU_SELECT_MESH			= 0x00000200,
	_MENU_SELECT_ANIMATION		= 0x00000400,
	_MENU_SELECT_TEXTURE		= 0x00000800,
	_MENU_RESOURCE_MASK			= 0x0000ff00,

	_MENU_SET_BACKGROUND		= 0x00010000,
	_MENU_SET_AMBIENT			= 0x00020000,
	_MENU_SET_DIRECTIONAL		= 0x00040000,
	_MENU_COLOR_MASK			= 0x00ff0000

} _Menu_e;

typedef enum
{
	//// Common.
	//
	_ACTION_TOGGLE_MAINMENU,					// FPADIO_INPUT_START
	_ACTION_TOGGLE_HELP,						// FPADIO_INPUT_CROSS_TOP
	_ACTION_BACK_TO_PREVIOUS,					// FPADIO_INPUT_CROSS_RIGHT
	//
	////

	//// Menu.
	//
	_ACTION_USE_SELECTION,						// FPADIO_INPUT_CROSS_BOTTOM

	_ACTION_SELECT_UP_DOWN1,					// FPADIO_INPUT_DPAD_Y
	_ACTION_SELECT_UP_DOWN2,					// FPADIO_INPUT_STICK_LEFT_Y
	_ACTION_SELECT_UP_DOWN3,					// FPADIO_INPUT_STICK_RIGHT_Y

	_ACTION_SELECT_LEFT_RIGHT1,					// FPADIO_INPUT_DPAD_X
	_ACTION_SELECT_LEFT_RIGHT2,					// FPADIO_INPUT_STICK_LEFT_X
	_ACTION_SELECT_LEFT_RIGHT3,					// FPADIO_INPUT_STICK_RIGHT_X

	_ACTION_SELECT_FILTER_UP_DOWN,				// FPADIO_INPUT_STICK_RIGHT_Y
	_ACTION_SELECT_FILTER_LEFT_RIGHT,			// FPADIO_INPUT_STICK_RIGHT_X
	_ACTION_SELECT_RESET_FILTER,				// FPADIO_INPUT_CROSS_LEFT
	_ACTION_SELECT_RESET_FILTER_LETTER,			// FPADIO_INPUT_TRIGGER_LEFT
	//
	////

	//// Viewing.
	//
	_ACTION_CYCLE_VIEW,							// FPADIO_INPUT_CROSS_BOTTOM
	_ACTION_CYCLE_MOVEMENT,						// FPADIO_INPUT_CROSS_LEFT

	_ACTION_PAN_LEFT_RIGHT1,					// FPADIO_INPUT_STICK_LEFT_X
	_ACTION_PAN_LEFT_RIGHT2,					// FPADIO_INPUT_DPAD_X
	_ACTION_HOVER_UP,							// FPADIO_INPUT_TRIGGER_RIGHT
	_ACTION_HOVER_DOWN,							// FPADIO_INPUT_TRIGGER_LEFT
	_ACTION_MOVE_FORWARD_BACKWARD1,				// FPADIO_INPUT_STICK_LEFT_Y
	_ACTION_MOVE_FORWARD_BACKWARD2,				// FPADIO_INPUT_DPAD_Y
	_ACTION_HEADING_LEFT_RIGHT,					// FPADIO_INPUT_STICK_RIGHT_X
	_ACTION_PITCH_UP_DOWN,						// FPADIO_INPUT_STICK_RIGHT_Y
	//
	////

	_ACTION_COUNT

} _Actions_e;

static FPadio_InputID_e _aoeInputID[ _ACTION_COUNT ] = {
															//// Common.
															//
															FPADIO_INPUT_START,
															FPADIO_INPUT_CROSS_TOP,
															FPADIO_INPUT_CROSS_RIGHT,
															//
															////

															//// Menu.
															//
															FPADIO_INPUT_CROSS_BOTTOM,

															FPADIO_INPUT_DPAD_Y,
															FPADIO_INPUT_STICK_LEFT_Y,
															FPADIO_INPUT_STICK_RIGHT_Y,

															FPADIO_INPUT_DPAD_X,
															FPADIO_INPUT_STICK_LEFT_X,
															FPADIO_INPUT_STICK_RIGHT_X,

															FPADIO_INPUT_STICK_RIGHT_Y,
															FPADIO_INPUT_STICK_RIGHT_X,
															FPADIO_INPUT_CROSS_LEFT,
															FPADIO_INPUT_TRIGGER_LEFT,
															//
															////

															//// Viewing.
															//
															FPADIO_INPUT_CROSS_BOTTOM,
															FPADIO_INPUT_CROSS_LEFT,

															FPADIO_INPUT_STICK_LEFT_X,
															FPADIO_INPUT_DPAD_X,
															FPADIO_INPUT_TRIGGER_RIGHT,
															FPADIO_INPUT_TRIGGER_LEFT,
															FPADIO_INPUT_STICK_LEFT_Y,
															FPADIO_INPUT_DPAD_Y,
															FPADIO_INPUT_STICK_RIGHT_X,
															FPADIO_INPUT_STICK_RIGHT_Y
															//
															////
														};

typedef struct
{
	//// Resources.
	//
	char szWorldFilter[ _RESOURCE_NAME_LENGTH + 1 ], szMeshFilter[ _RESOURCE_NAME_LENGTH + 1 ], szAnimationFilter[ _RESOURCE_NAME_LENGTH + 1 ], szTextureFilter[ _RESOURCE_NAME_LENGTH + 1 ];
	char szWorldSelection[ _RESOURCE_NAME_LENGTH + 1 ], szMeshSelection[ _RESOURCE_NAME_LENGTH + 1 ], szAnimationSelection[ _RESOURCE_NAME_LENGTH + 1 ], szTextureSelection[ _RESOURCE_NAME_LENGTH + 1 ];
	//
	////

	//// Lights.
	//
	f32 afColorBackgroundRGBI[ 4 ], afColorAmbientRGBI[ 4 ], afColorDirectionalRGBI[ 4 ],
		afWSCameraXYZHP[ 5 ], afWSDirectionalXYZ[ 3 ], afWSDirectionalHP[ 2 ];
	//
	////

	//// Current subject.
	//
	_ResourceTypeID_e oeResourceID;
	char szResourceName[ _RESOURCE_NAME_LENGTH + 1 ];
	char szAnimationName[ _RESOURCE_NAME_LENGTH + 1 ];
	//
	////

} _Settings_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Common.
//static f32 _fXSpeed, _fYSpeed, _fHSpeed, _fPSpeed;
static f32 _fZSpeed;
static BOOL _bShowMenu, _bShowHelp, _bShowError, _bPreviousShowError, _bBackPressed, _bRestoreStates;

// Menus.
static FDrawVtx_t _aoBackgroundVerts[ 4 ];
static struct
{
	BOOL bSelect, bUp, bDown, bAlternateUp, bAlternateDown, bLeft, bRight, bAlternateLeft, bAlternateRight, bFilterErase, bFilterEraseLetter, bFilterUp, bFilterDown, bFilterLeft, bFilterRight;
/*
	f32 fDelayUp, bDelayDown, bDelayLeft, bDelayRight, bDelayFilterErase, bDelayFilterUp, bDelayFilterDown, bDelayFilterLeft, bDelayFilterRight;
	f32 fWaitedUp, bWaitedDown, bWaitedLeft, bWaitedRight, bWaitedFilterErase, bWaitedFilterUp, bWaitedFilterDown, bWaitedFilterLeft, bWaitedFilterRight;
*/
} _oMenuNavigation;

static char _szFilterChars[ _FILTER_NUM_CHARS_IN_SET + 1 ] = { "?abcdefghijklmnopqrstuvwxyz0123456789_" };
static u32 _uFirstIndexOnCurrentPage;
static s32 _nFilterPosition, _nFilterCharIndex;

static char _szErrorMessage[ 63 + 1 ];
static void *_pCurrentSubject;
static char *_pszCurrentSubjectName;
static char **_paszCurrentResourceList;
static u32 _uCurrentResourceListLength;
static _Menu_e _eCurrentMenu;
static s32 _nMainMenuSelection, _nResourceMenuSelection, _nColorMenuSelection;
static CFTexInst _oTexInst;

static FDrawVtx_t _aoColorSampleVerts[ 4 ];

static char *_pszCurrentFilter, *_pszCurrentSelection;
static f32 *_pafCurrentColor;
static CFXfm _oBackgroundXfm;

// Subject.
static _MovementTypeID_e _eMovementType;
static u32 _uStartpoints, _uCurrentStartpoint;
static struct
{
	BOOL bCycleView, bCycleMove;
	f32 fX, fY, fZ, fHeading, fPitch;

} _oSubjectNavigation;

// Display.
static FViewport_t *_poMainViewport, *_poBackgroundViewport, *_poArrowViewport;
static CFXfm _oCameraXfm;
static CFLight _oDirectionalLight;
static FDrawVtx_t _aoDirLightOrientationArrowVerts[ 60 ];
static FDrawVtx_t _aoDirLightOrientationArrowOutlineVerts[ 28 ];
static CFXfm _oArrowXfm;

// Meshes.
static u32 _uMeshes;
static char **_papszMeshes;
static CFMeshInst _oMeshInst;
static CFXfm _oMeshXfm;
//rename..
static CFMtx43 _oMtx43Frame;
static CFVec3 _oVec3Frame;

// Filter.
static u32 _uFiltered;
static char **_papszFiltered;

// Animations.
static u32 _uAnimations;
static char **_papszAnimations;
static CFAnimInst *_poAnimInst;
static CFAnimCombiner *_poAnimCombiner;
static FAnim_t *_poCurrentAnimation;
static char *_pszCurrentAnimationName;

// Worlds.
static u32 _uWorlds;
static char **_papszWorlds;

// Textures.
static u32 _uTextures;
static char **_papszTextures;
static FDrawVtx_t _aoTextureVerts[ 4 ];

// Pad.
static u32 _uCurrentPadPlusOne, _uPadConnected, _uPadConnectedPreviously;
static FPad_Sample_t *_apoSamples[ _ACTION_COUNT ];
static f32 _fPadErrorMessageDelay;

// Storage.
static u32 _uCurrentStorage, _uStorageConnected, _uStorageInserted, _uStorageRemoved;
static BOOL _bValid;
static _Settings_t _oSettings;

// Memory.
static FResFrame_t _ohResFrameSubject;
static FResFrame_t _ohResFrameAnim;
static FMemFrame_t _ohMemFrame;

// Temp.
static f32 _fTemp;
static u32 _uIndex, _uTemp;
static char **_pszTempString, _szTempOutput[ 5120 ], _szTempLine[ 120 ];
static CFMtx43 _oTempMtx43;
static CFVec3 _oTempVec3, _oTempVec3MS, _oTempVec3WS;

#if _AUDIO_TEST
	static FAudio_WaveHandle_t _aohWaveHandles[ 7 ];
	static CFAudioStream *_apoAudioStream[ 4 ];

	static CFTimer _oPerformaceTimer;
	static f32 _fMilisecondsTemp, _fUsageTotMiliseconds, _fUsageMaxMiliseconds, _fUsageMinMiliseconds;
	static f32 _fUsageReportDelay;
	static u32 _uTimeStamp;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _ViewerInit( void *pParameter );
static void _ViewerTerm( FLoopTermCode_t nTermCode, void *pParameter );
static BOOL _ViewerMain( BOOL bExitRequest, void *pParameter );
static void _ManagePad( void );
static BOOL _LoadSettings( void );
static BOOL _SaveSettings( void );
static void _ManageStorage( void );
static void _ManageSubject( void );
static void _ManageMenu( void );
static void _DisplaySubject( void );
static void _DisplayMenu( void );
static void _DisplayError( void );
static void _PrintMainMenu( void );
static void _PrintResourceMenu( void );
static void _PrintColorMenu( void );
static void _SetDefaultDirLight( void );
static void _ProcessPadStatesSubject( void );
static void _ProcessPadStatesMenu( void );
static void _ManageMainMenu( void );
static void _ManageResourceMenu( void );
static void _ManageColorMenu( void );
static void _ManageWorldSubject( void );
static void _ManageMeshSubject( void );
static void _ManageTextureSubject( void );
static void _BuildFilteredList( void );
static void _FindCurrentSelectionIndex( void );
static void _CountStartPoint( void );
static void _MoveCameraToNextStartPoint( void );
static void _PrintHelpMainMenu( void );
static void _PrintHelpResourceMenu( void );
static void _PrintHelpColorMenu( void );
static void _PrintHelpWorldDisplay( void );
static void _PrintHelpMeshDisplay( void );
static void _PrintHelpTextureDisplay( void );
static void _ProcessPadStatesCommon( void );
static void _DisplayHelp( void );

//ARG - >>>>>
#if FANG_PLATFORM_PS2
static void PS2EditFilter(void);
#endif
//ARG - <<<<<

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL viewer_Start( ViewerInitParm_t *pParms )
{
	FASSERT( fang_IsStartedUp() );
	FASSERT( pParms );

	return floop_InstallGameloop( _ViewerInit, _ViewerMain, _ViewerTerm, pParms, 60, 15 );

} // viewer_Start

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _ViewerInit( void *pParameter )
{
	ViewerInitParm_t *pParm = (ViewerInitParm_t *)pParameter;

	u32 uCount;

	//// Graphics.
	//
	if( ! fvid_CreateWindow( &pParm->VidWin ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fvid_CreateWindow() failed.\nTry a different video mode.", __LINE__ );
		return FALSE;
	}

//	floop_SetTargetFramesPerSec( 60 );
//	floop_EnableFixedLoopTimeMode( TRUE );

	_poMainViewport = fviewport_Create();
	if( _poMainViewport == NULL )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fviewport_Create() failed.", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}
	fviewport_InitPersp( _poMainViewport, FMATH_QUARTER_PI, _NEARPLANE, _FARPLANE );

	_poBackgroundViewport = fviewport_Create();
	if( _poBackgroundViewport == NULL )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fviewport_Create() failed.", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}
	fviewport_InitOrtho2D( _poBackgroundViewport, _NEARPLANE, _FARPLANE );

	_poArrowViewport = fviewport_Create();
	if( _poArrowViewport == NULL )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fviewport_Create() failed.", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}
	fviewport_InitOrtho3D( _poArrowViewport, _NEARPLANE, _FARPLANE );

	_oCameraXfm.Identity();

	fworld_EnableFog( TRUE );
	//
	////

	//// Sub-systems.
	//
	FPadio_Init_t oPadioInit;
	fang_MemZero( &oPadioInit, sizeof( oPadioInit ) );

#if FANG_PLATFORM_WIN
	oPadioInit.DX8ONLY_ohWnd = (u32)fdx8vid_GetWindowHandle();
	oPadioInit.DX8ONLY_ohInstance = (u32)FVid_Win.hInstance;
/*
	oPadioInit.DX8ONLY_pauInputEmulationMap = pParm->pauInputEmulationMap;
	switch( Gameloop_nPlatform ) {
	default:
	case GAMELOOP_PLATFORM_XB:
		oPadioInit.DX8ONLY_uInputEmulationPlatform = FPADIO_INPUT_EMULATION_PLATFORM_XB;
		break;
	case GAMELOOP_PLATFORM_GC:
		oPadioInit.DX8ONLY_uInputEmulationPlatform = FPADIO_INPUT_EMULATION_PLATFORM_GC;
		break;
	}
	oPadioInit.DX8ONLY_pszInputEmulationDevName = pParm->pszInputEmulationDevName;
*/
#endif

#if _AUDIO_TEST
	FAudio_Init_t oAudioInit;
	FAudio_PriorityLimit_t aoInfo[ 2 ] = { { 1, 2 }, { 3, 2 } };

	oAudioInit.uMaxBanks = 4;
	oAudioInit.uMaxEmitters = 16;
	oAudioInit.uMaxStreams = 4;
	oAudioInit.uMaxListeners = 1;
	oAudioInit.uMaxPriorityLevels = 5;

	oAudioInit.uPriorityLimits = 2;
	oAudioInit.paoPriorityLimits = aoInfo;

	if( FAUDIO_NO_ERROR != faudio_Install( &oAudioInit ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: faudio_Install() failed.\n", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}

	char aszBanks1[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ] = "ms9_lvl_02";
	char aszBanks2[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ] = "ms9_blink";
	char aszBanks3[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ] = "ms9_bots";
	char aszBanks4[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ] = "ms9_weapons";

	char *apszBanks[ 5 ] = { aszBanks1, aszBanks2, aszBanks3, aszBanks4, NULL };
	FAudio_BankHandle_t aoBankHandles[ 4 ];

	faudio_LoadBanks( (cchar **)apszBanks, aoBankHandles );

	_aohWaveHandles[ 0 ] = faudio_GetWaveHandle( aoBankHandles[ 0 ], "crunch3" );
	_aohWaveHandles[ 1 ] = faudio_GetWaveHandle( aoBankHandles[ 0 ], "sef1_comp04" );
	_aohWaveHandles[ 2 ] = faudio_GetWaveHandle( aoBankHandles[ 0 ], "sewer2" );
	_aohWaveHandles[ 3 ] = faudio_GetWaveHandle( aoBankHandles[ 0 ], "swdm_gxpl04" );
	_aohWaveHandles[ 4 ] = faudio_GetWaveHandle( aoBankHandles[ 1 ], "swmt_rled01" );
	_aohWaveHandles[ 5 ] = faudio_GetWaveHandle( aoBankHandles[ 2 ], "srmt_deth02" );
	_aohWaveHandles[ 6 ] = faudio_GetWaveHandle( aoBankHandles[ 3 ], "blaster" );

	FAudio_WaveInfo_t oWaveInfo;
	faudio_GetWaveAttributes( _aohWaveHandles[ 0 ], &oWaveInfo );
	faudio_GetWaveAttributes( _aohWaveHandles[ 1 ], &oWaveInfo );
	faudio_GetWaveAttributes( _aohWaveHandles[ 2 ], &oWaveInfo );

	_fUsageTotMiliseconds = 0.0f;
	_fUsageMaxMiliseconds = 0.0f;
	_fUsageMinMiliseconds = 1000000000.0f;
	_uTimeStamp           = 0;
	_fUsageReportDelay    = 0.0f;

	////
	//
	_apoAudioStream[ 0 ] = CFAudioStream::Create( "SpiderMan" );
	_apoAudioStream[ 1 ] = CFAudioStream::Create( "pulp" );

	_apoAudioStream[ 2 ] = CFAudioStream::Create( "GetATaco" );
	_apoAudioStream[ 3 ] = CFAudioStream::Create( "seinfeld" );
	//
	////
#endif

	if( FTEXT_NO_ERROR != ftext_Install() )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: ftext_Install() failed.\n", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}

	FStorage_Init_t oStorageInit;
	fang_MemZero( &oStorageInit, sizeof( oStorageInit ) );

#if FANG_PLATFORM_GC
	u8 auIconC8andCLUT[ ( 32 * 32 ) + ( 2 * 256 ) ];
	fang_MemZero( auIconC8andCLUT, sizeof( auIconC8andCLUT ) );
	oStorageInit.GCONLY_puIcon = auIconC8andCLUT;
	oStorageInit.GCONLY_uIconSize = sizeof( auIconC8andCLUT );
#endif

	if( FSTORAGE_ERROR_NONE != fstorage_Install( &oStorageInit ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fstorage_Install() failed.\n", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}

	_uCurrentStorage = 0;

	if( FPADIO_NO_ERROR != fpadio_Install( &oPadioInit ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fpadio_Install() failed.\n", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}

	if( FPAD_NO_ERROR != fpad_Install( 0.5f, 0.05f, 0.6f, 0.7f ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fpad_Install() failed.\n", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}

	_uCurrentPadPlusOne = 0;
	_uPadConnectedPreviously = 0;
	_fPadErrorMessageDelay = 0.0f;
	fang_MemZero( &_oSettings, sizeof( _oSettings ) );
	fang_MemSet( _oSettings.szWorldFilter,     _FILTER_ANY_CHAR, _RESOURCE_NAME_LENGTH );
	fang_MemSet( _oSettings.szMeshFilter,      _FILTER_ANY_CHAR, _RESOURCE_NAME_LENGTH );
	fang_MemSet( _oSettings.szAnimationFilter, _FILTER_ANY_CHAR, _RESOURCE_NAME_LENGTH );
	fang_MemSet( _oSettings.szTextureFilter,   _FILTER_ANY_CHAR, _RESOURCE_NAME_LENGTH );
	//
	////

	//// Masterfile.
	//
	//// Validate.
	//
	if( ! ffile_IsInMasterMode() )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: ffile_IsInMasterMode() failed.\n", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}

	FDataPrjFile_Header_t oHeader;

	FFileHandle oFileHandle = ffile_Open( Fang_ConfigDefs.pszFile_MasterFilePathName, FFILE_OPEN_RONLY, TRUE );
	if( ! oFileHandle )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: ffile_Open() failed.\n", __LINE__ );
		fvid_DestroyWindow();
		return FALSE;
	}

	if( ffile_Read( oFileHandle, sizeof( FDataPrjFile_Header_t ), &oHeader ) != sizeof( FDataPrjFile_Header_t ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: ffile_Read() failed.\n", __LINE__ );
		ffile_Close( oFileHandle );
		fvid_DestroyWindow();
		return FALSE;
	}

	if( oHeader.Version.nSignature != FVERSION_FILE_SIGNATURE )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: Signatures do not match.\n", __LINE__ );
		ffile_Close( oFileHandle );
		fvid_DestroyWindow();
		return FALSE;
	}

	if( ( FVERSION_GET_MAJOR_NUMBER( oHeader.Version.nVersion ) != FDATA_PRJFILE_MAJOR ) ||
		( FVERSION_GET_MINOR_NUMBER( oHeader.Version.nVersion ) != FDATA_PRJFILE_MINOR ) ||
		( FVERSION_GET_SUB_NUMBER(   oHeader.Version.nVersion ) != FDATA_PRJFILE_SUB ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: Version numbers do not match.\n", __LINE__ );
		ffile_Close( oFileHandle );
		fvid_DestroyWindow();
		return FALSE;
	}

	ffile_Seek( oFileHandle, 0, FFILE_SEEK_END );
	if( ffile_Tell( oFileHandle ) != (int)oHeader.nBytesInFile )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: File sizes do not match.\n", __LINE__ );
		ffile_Close( oFileHandle );
		fvid_DestroyWindow();
		return FALSE;
	}
	ffile_Seek( oFileHandle, sizeof( FDataPrjFile_Header_t ), FFILE_SEEK_SET );
	//
	////

	//// Read.
	//
	_ohMemFrame = fmem_GetFrame();

	FDataPrjFile_Entry_t *poEntries = (FDataPrjFile_Entry_t *)fmem_Alloc( oHeader.nNumEntries * sizeof( FDataPrjFile_Entry_t ) );
	if( ! poEntries )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fmem_Alloc() failed.\n", __LINE__ );
		ffile_Close( oFileHandle );
		fvid_DestroyWindow();
		return FALSE;
	}

	if( ffile_Read( oFileHandle, ( oHeader.nNumEntries * sizeof( FDataPrjFile_Entry_t ) ), poEntries ) != ( oHeader.nNumEntries * sizeof( FDataPrjFile_Entry_t ) ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: ffile_Read() failed.\n", __LINE__ );
		_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
		ffile_Close( oFileHandle );
		fvid_DestroyWindow();
		return FALSE;
	}

	ffile_Close( oFileHandle );
	//
	////

	//// Count.
	//
	_uMeshes = _uAnimations = _uWorlds = _uTextures = 0;

	for( uCount = 0; uCount < oHeader.nNumEntries; ++uCount )
	{
		if( strstr( poEntries[ uCount ].szFilename, ".ape" ) ) // Mesh.
		{
			++_uMeshes;
		}
		else if( strstr( poEntries[ uCount ].szFilename, ".mtx" ) ) // Animation.
		{
			++_uAnimations;
		}
		else if( strstr( poEntries[ uCount ].szFilename, ".wld" ) ) // World.
		{
			++_uWorlds;
		}
		else if( strstr( poEntries[ uCount ].szFilename, ".tga" ) ) // Textures.
		{
			++_uTextures;
		}
	}
	//
	////

	//// Allocate memory to hold names.
	//
	_uFiltered = FMATH_MAX( FMATH_MAX( _uWorlds, _uMeshes ), FMATH_MAX( _uAnimations, _uTextures ) );

	_ohResFrameSubject = fres_GetFrame();

	_papszMeshes     = (char **)fres_Alloc( sizeof( char * ) * _uMeshes );
	_papszAnimations = (char **)fres_Alloc( sizeof( char * ) * _uAnimations );
	_papszWorlds     = (char **)fres_Alloc( sizeof( char * ) * _uWorlds );
	_papszTextures   = (char **)fres_Alloc( sizeof( char * ) * _uTextures );
	_papszFiltered   = (char **)fres_Alloc( sizeof( char * ) * _uFiltered );

	if( ! ( _papszMeshes && _papszAnimations && _papszWorlds && _papszTextures && _papszFiltered ) )
	{
		DEVPRINTF( "[ VIEWER ] Error %u: fres_Alloc() failed.\n", __LINE__ );
		_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
		_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
		fvid_DestroyWindow();
		return FALSE;
	}

	for( uCount = 0; uCount < _uMeshes; ++uCount )
	{
		_papszMeshes[ uCount ] = (char *)fres_AllocAndZero( FDATA_MESHNAME_LEN + 1 );
		if( _papszMeshes[ uCount ] )
		{
			continue;
		}

		DEVPRINTF( "[ VIEWER ] Error %u: fres_Alloc() failed.\n", __LINE__ );
		_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
		_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
		fvid_DestroyWindow();
		return FALSE;
	}

	for( uCount = 0; uCount < _uAnimations; ++uCount )
	{
		_papszAnimations[ uCount ] = (char *)fres_AllocAndZero( FDATA_MESHNAME_LEN + 1 );
		if( _papszAnimations[ uCount ] )
		{
			continue;
		}

		DEVPRINTF( "[ VIEWER ] Error %u: fres_Alloc() failed.\n", __LINE__ );
		_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
		_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
		fvid_DestroyWindow();
		return FALSE;
	}

	for( uCount = 0; uCount < _uWorlds; ++uCount )
	{
		_papszWorlds[ uCount ] = (char *)fres_AllocAndZero( FDATA_MESHNAME_LEN + 1 );
		if( _papszWorlds[ uCount ] )
		{
			continue;
		}

		DEVPRINTF( "[ VIEWER ] Error %u: fres_Alloc() failed.\n", __LINE__ );
		_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
		_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
		fvid_DestroyWindow();
		return FALSE;
	}

	for( uCount = 0; uCount < _uTextures; ++uCount )
	{
		_papszTextures[ uCount ] = (char *)fres_AllocAndZero( FDATA_MESHNAME_LEN + 1 );
		if( _papszTextures[ uCount ] )
		{
			continue;
		}

		DEVPRINTF( "[ VIEWER ] Error %u: fres_Alloc() failed.\n", __LINE__ );
		_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
		_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
		fvid_DestroyWindow();
		return FALSE;
	}
	//
	////

	//// Copy names.
	//
	_uMeshes = _uAnimations = _uWorlds = _uTextures = 0;

	char *pszTemp;

	for( uCount = 0; uCount < oHeader.nNumEntries; ++uCount )
	{
		pszTemp = strstr( poEntries[ uCount ].szFilename, ".ape" ); // Mesh.
		if( pszTemp )
		{
			*pszTemp = NULL;
			fclib_strcpy( _papszMeshes[ _uMeshes ], poEntries[ uCount ].szFilename );
			++_uMeshes;
			continue;
		}

		pszTemp = strstr( poEntries[ uCount ].szFilename, ".mtx" ); // Animation.
		if( pszTemp )
		{
			*pszTemp = NULL;
			fclib_strcpy( _papszAnimations[ _uAnimations ], poEntries[ uCount ].szFilename );
			++_uAnimations;
			continue;
		}

		pszTemp = strstr( poEntries[ uCount ].szFilename, ".wld" ); // World.
		if( pszTemp )
		{
			*pszTemp = NULL;
			fclib_strcpy( _papszWorlds[ _uWorlds ], poEntries[ uCount ].szFilename );
			++_uWorlds;
			continue;
		}

		pszTemp = strstr( poEntries[ uCount ].szFilename, ".tga" ); // Textures.
		if( pszTemp )
		{
			*pszTemp = NULL;
			fclib_strcpy( _papszTextures[ _uTextures ], poEntries[ uCount ].szFilename );
			++_uTextures;
			continue;
		}
	}
	//
	////

	_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
	_ohResFrameSubject = NULL;
	//
	//// Masterfile.

	////
	//
	_oSettings.afColorAmbientRGBI[ 0 ] = _DEFAULT_AMBIENT_LIGHT_R;
	_oSettings.afColorAmbientRGBI[ 1 ] = _DEFAULT_AMBIENT_LIGHT_G;
	_oSettings.afColorAmbientRGBI[ 2 ] = _DEFAULT_AMBIENT_LIGHT_B;
	_oSettings.afColorAmbientRGBI[ 3 ] = _DEFAULT_AMBIENT_LIGHT_I;

	_oSettings.afColorBackgroundRGBI[ 0 ] = _DEFAULT_BACKGROUND_COLOR_R;
	_oSettings.afColorBackgroundRGBI[ 1 ] = _DEFAULT_BACKGROUND_COLOR_G;
	_oSettings.afColorBackgroundRGBI[ 2 ] = _DEFAULT_BACKGROUND_COLOR_B;
	_oSettings.afColorBackgroundRGBI[ 3 ] = _DEFAULT_BACKGROUND_COLOR_I;

	_oSettings.afColorDirectionalRGBI[ 0 ] = _DEFAULT_DIRECTIONAL_LIGHT_R;
	_oSettings.afColorDirectionalRGBI[ 1 ] = _DEFAULT_DIRECTIONAL_LIGHT_G;
	_oSettings.afColorDirectionalRGBI[ 2 ] = _DEFAULT_DIRECTIONAL_LIGHT_B;
	_oSettings.afColorDirectionalRGBI[ 3 ] = _DEFAULT_DIRECTIONAL_LIGHT_I;

	_SetDefaultDirLight();
	//
	////

	FTEXT_SET_DEBUG_COLOR( ( 59.0f / 99.0f ), ( 59.0f / 99.0f ), 0.0f, 1.0f );

	fang_MemZero( _aoBackgroundVerts, sizeof( _aoBackgroundVerts ) );
	_aoBackgroundVerts[ 0 ].ColorRGBA.Set( 0.0f, _MENU_BACKGROUND_ALPHA );
	_aoBackgroundVerts[ 1 ].ColorRGBA.Set( 0.0f, _MENU_BACKGROUND_ALPHA );
	_aoBackgroundVerts[ 2 ].ColorRGBA.Set( 0.0f, _MENU_BACKGROUND_ALPHA );
	_aoBackgroundVerts[ 3 ].ColorRGBA.Set( 0.0f, _MENU_BACKGROUND_ALPHA );
	_aoBackgroundVerts[ 0 ].Pos_MS.Set( 0.0f, 0.0f, 1.0f );
	_aoBackgroundVerts[ 1 ].Pos_MS.Set( _poBackgroundViewport->Res.x, 0.0f, 1.0f );
	_aoBackgroundVerts[ 2 ].Pos_MS.Set( 0.0f, _poBackgroundViewport->Res.y, 1.0f );
	_aoBackgroundVerts[ 3 ].Pos_MS.Set( _poBackgroundViewport->Res.x, _poBackgroundViewport->Res.y, 1.0f );

	fang_MemZero( _aoTextureVerts, sizeof( _aoTextureVerts ) );
	_aoTextureVerts[ 0 ].ST.Set( 0.0f, 0.0f );
	_aoTextureVerts[ 1 ].ST.Set( 1.0f, 0.0f );
	_aoTextureVerts[ 2 ].ST.Set( 0.0f, 1.0f );
	_aoTextureVerts[ 3 ].ST.Set( 1.0f, 1.0f );
	_aoTextureVerts[ 0 ].ColorRGBA.OpaqueWhite();
	_aoTextureVerts[ 1 ].ColorRGBA.OpaqueWhite();
	_aoTextureVerts[ 2 ].ColorRGBA.OpaqueWhite();
	_aoTextureVerts[ 3 ].ColorRGBA.OpaqueWhite();

	fang_MemZero( _aoColorSampleVerts, sizeof( _aoColorSampleVerts ) );
	_aoColorSampleVerts[ 0 ].Pos_MS.Set( ( _poBackgroundViewport->Res.x * _MENU_COLOR_SAMPLE_BOX_LEFT  ), ( _poBackgroundViewport->Res.y * _MENU_COLOR_SAMPLE_BOX_TOP    ), ( _NEARPLANE + 0.1f ) );
	_aoColorSampleVerts[ 1 ].Pos_MS.Set( ( _poBackgroundViewport->Res.x * _MENU_COLOR_SAMPLE_BOX_RIGHT ), ( _poBackgroundViewport->Res.y * _MENU_COLOR_SAMPLE_BOX_TOP    ), ( _NEARPLANE + 0.1f ) );
	_aoColorSampleVerts[ 2 ].Pos_MS.Set( ( _poBackgroundViewport->Res.x * _MENU_COLOR_SAMPLE_BOX_LEFT  ), ( _poBackgroundViewport->Res.y * _MENU_COLOR_SAMPLE_BOX_BOTTOM ), ( _NEARPLANE + 0.1f ) );
	_aoColorSampleVerts[ 3 ].Pos_MS.Set( ( _poBackgroundViewport->Res.x * _MENU_COLOR_SAMPLE_BOX_RIGHT ), ( _poBackgroundViewport->Res.y * _MENU_COLOR_SAMPLE_BOX_BOTTOM ), ( _NEARPLANE + 0.1f ) );

	////
	//
	fang_MemZero( _aoDirLightOrientationArrowVerts, sizeof( _aoDirLightOrientationArrowVerts ) );
	for( _uIndex = 0; _uIndex < 60; ++_uIndex )
	{
		_aoDirLightOrientationArrowVerts[ _uIndex ].ColorRGBA.Set( _DIRECTIONAL_LIGHT_ARROW_R, _DIRECTIONAL_LIGHT_ARROW_G, _DIRECTIONAL_LIGHT_ARROW_B, _DIRECTIONAL_LIGHT_ARROW_A );
	}
/*
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set( -1.0f, -0.5f, -2.0f ); // A
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set( -1.0f, -0.5f,  0.0f ); // B
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set( -2.0f, -0.5f,  0.0f ); // C
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  0.0f, -0.5f,  2.0f ); // D
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  2.0f, -0.5f,  0.0f ); // E
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  1.0f, -0.5f,  0.0f ); // F
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  1.0f, -0.5f, -2.0f ); // G
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set( -1.0f,  0.5f, -2.0f ); // H
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set( -1.0f,  0.5f,  0.0f ); // I
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set( -2.0f,  0.5f,  0.0f ); // J
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  0.0f,  0.5f,  2.0f ); // K
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  2.0f,  0.5f,  0.0f ); // L
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  1.0f,  0.5f,  0.0f ); // M
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set(  1.0f,  0.5f, -2.0f ); // N
*/
	_aoDirLightOrientationArrowVerts[  0 ].Pos_MS.Set( -2.0f,  0.5f,  0.0f ); // J
	_aoDirLightOrientationArrowVerts[  1 ].Pos_MS.Set(  0.0f,  0.5f,  2.0f ); // K
	_aoDirLightOrientationArrowVerts[  2 ].Pos_MS.Set(  2.0f,  0.5f,  0.0f ); // L

	_aoDirLightOrientationArrowVerts[  3 ].Pos_MS.Set(  0.0f,  0.5f,  2.0f ); // K
	_aoDirLightOrientationArrowVerts[  4 ].Pos_MS.Set(  2.0f,  0.5f,  0.0f ); // L
	_aoDirLightOrientationArrowVerts[  5 ].Pos_MS.Set(  0.0f, -0.5f,  2.0f ); // D

	_aoDirLightOrientationArrowVerts[  6 ].Pos_MS.Set(  0.0f, -0.5f,  2.0f ); // D
	_aoDirLightOrientationArrowVerts[  7 ].Pos_MS.Set(  2.0f,  0.5f,  0.0f ); // L
	_aoDirLightOrientationArrowVerts[  8 ].Pos_MS.Set(  2.0f, -0.5f,  0.0f ); // E

	_aoDirLightOrientationArrowVerts[  9 ].Pos_MS.Set(  2.0f, -0.5f,  0.0f ); // E
	_aoDirLightOrientationArrowVerts[ 10 ].Pos_MS.Set(  2.0f,  0.5f,  0.0f ); // L
	_aoDirLightOrientationArrowVerts[ 11 ].Pos_MS.Set(  1.0f, -0.5f,  0.0f ); // F

	_aoDirLightOrientationArrowVerts[ 12 ].Pos_MS.Set(  2.0f,  0.5f,  0.0f ); // L
	_aoDirLightOrientationArrowVerts[ 13 ].Pos_MS.Set(  1.0f, -0.5f,  0.0f ); // F
	_aoDirLightOrientationArrowVerts[ 14 ].Pos_MS.Set(  1.0f,  0.5f,  0.0f ); // M

	_aoDirLightOrientationArrowVerts[ 15 ].Pos_MS.Set(  1.0f, -0.5f,  0.0f ); // F
	_aoDirLightOrientationArrowVerts[ 16 ].Pos_MS.Set(  1.0f,  0.5f,  0.0f ); // M
	_aoDirLightOrientationArrowVerts[ 17 ].Pos_MS.Set(  1.0f,  0.5f, -2.0f ); // N

	_aoDirLightOrientationArrowVerts[ 18 ].Pos_MS.Set(  1.0f, -0.5f,  0.0f ); // F
	_aoDirLightOrientationArrowVerts[ 19 ].Pos_MS.Set(  1.0f,  0.5f, -2.0f ); // N
	_aoDirLightOrientationArrowVerts[ 20 ].Pos_MS.Set(  1.0f, -0.5f, -2.0f ); // G

	_aoDirLightOrientationArrowVerts[ 21 ].Pos_MS.Set(  1.0f, -0.5f,  0.0f ); // F
	_aoDirLightOrientationArrowVerts[ 22 ].Pos_MS.Set(  1.0f, -0.5f, -2.0f ); // G
	_aoDirLightOrientationArrowVerts[ 23 ].Pos_MS.Set( -1.0f, -0.5f,  0.0f ); // B

	_aoDirLightOrientationArrowVerts[ 24 ].Pos_MS.Set( -1.0f, -0.5f,  0.0f ); // B
	_aoDirLightOrientationArrowVerts[ 25 ].Pos_MS.Set(  1.0f, -0.5f, -2.0f ); // G
	_aoDirLightOrientationArrowVerts[ 26 ].Pos_MS.Set( -1.0f, -0.5f, -2.0f ); // A

	_aoDirLightOrientationArrowVerts[ 27 ].Pos_MS.Set( -1.0f, -0.5f, -2.0f ); // A
	_aoDirLightOrientationArrowVerts[ 28 ].Pos_MS.Set(  1.0f, -0.5f, -2.0f ); // G
	_aoDirLightOrientationArrowVerts[ 29 ].Pos_MS.Set(  1.0f,  0.5f, -2.0f ); // N

	_aoDirLightOrientationArrowVerts[ 30 ].Pos_MS.Set( -1.0f, -0.5f, -2.0f ); // A
	_aoDirLightOrientationArrowVerts[ 31 ].Pos_MS.Set(  1.0f,  0.5f, -2.0f ); // N
	_aoDirLightOrientationArrowVerts[ 32 ].Pos_MS.Set( -1.0f,  0.5f, -2.0f ); // H

	_aoDirLightOrientationArrowVerts[ 33 ].Pos_MS.Set(  1.0f,  0.5f, -2.0f ); // N
	_aoDirLightOrientationArrowVerts[ 34 ].Pos_MS.Set( -1.0f,  0.5f, -2.0f ); // H
	_aoDirLightOrientationArrowVerts[ 35 ].Pos_MS.Set(  1.0f,  0.5f,  0.0f ); // M

	_aoDirLightOrientationArrowVerts[ 36 ].Pos_MS.Set(  1.0f,  0.5f,  0.0f ); // M
	_aoDirLightOrientationArrowVerts[ 37 ].Pos_MS.Set( -1.0f,  0.5f, -2.0f ); // H
	_aoDirLightOrientationArrowVerts[ 38 ].Pos_MS.Set( -1.0f,  0.5f,  0.0f ); // I

	_aoDirLightOrientationArrowVerts[ 39 ].Pos_MS.Set( -1.0f,  0.5f, -2.0f ); // H
	_aoDirLightOrientationArrowVerts[ 40 ].Pos_MS.Set( -1.0f,  0.5f,  0.0f ); // I
	_aoDirLightOrientationArrowVerts[ 41 ].Pos_MS.Set( -1.0f, -0.5f, -2.0f ); // A

	_aoDirLightOrientationArrowVerts[ 42 ].Pos_MS.Set( -1.0f,  0.5f,  0.0f ); // I
	_aoDirLightOrientationArrowVerts[ 43 ].Pos_MS.Set( -1.0f, -0.5f, -2.0f ); // A
	_aoDirLightOrientationArrowVerts[ 44 ].Pos_MS.Set( -1.0f, -0.5f,  0.0f ); // B

	_aoDirLightOrientationArrowVerts[ 45 ].Pos_MS.Set( -1.0f,  0.5f,  0.0f ); // I
	_aoDirLightOrientationArrowVerts[ 46 ].Pos_MS.Set( -1.0f, -0.5f,  0.0f ); // B
	_aoDirLightOrientationArrowVerts[ 47 ].Pos_MS.Set( -2.0f,  0.5f,  0.0f ); // J

	_aoDirLightOrientationArrowVerts[ 48 ].Pos_MS.Set( -1.0f, -0.5f,  0.0f ); // B
	_aoDirLightOrientationArrowVerts[ 49 ].Pos_MS.Set( -2.0f,  0.5f,  0.0f ); // J
	_aoDirLightOrientationArrowVerts[ 50 ].Pos_MS.Set( -2.0f, -0.5f,  0.0f ); // C

	_aoDirLightOrientationArrowVerts[ 51 ].Pos_MS.Set( -2.0f,  0.5f,  0.0f ); // J
	_aoDirLightOrientationArrowVerts[ 52 ].Pos_MS.Set( -2.0f, -0.5f,  0.0f ); // C
	_aoDirLightOrientationArrowVerts[ 53 ].Pos_MS.Set(  0.0f,  0.5f,  2.0f ); // K

	_aoDirLightOrientationArrowVerts[ 54 ].Pos_MS.Set( -2.0f, -0.5f,  0.0f ); // C
	_aoDirLightOrientationArrowVerts[ 55 ].Pos_MS.Set(  0.0f,  0.5f,  2.0f ); // K
	_aoDirLightOrientationArrowVerts[ 56 ].Pos_MS.Set(  0.0f, -0.5f,  2.0f ); // D

	_aoDirLightOrientationArrowVerts[ 57 ].Pos_MS.Set( -2.0f, -0.5f,  0.0f ); // C
	_aoDirLightOrientationArrowVerts[ 58 ].Pos_MS.Set(  0.0f, -0.5f,  2.0f ); // D
	_aoDirLightOrientationArrowVerts[ 59 ].Pos_MS.Set(  2.0f, -0.5f,  0.0f ); // E

	_aoDirLightOrientationArrowOutlineVerts[  0 ] = _aoDirLightOrientationArrowVerts[ 26 ]; // A
	_aoDirLightOrientationArrowOutlineVerts[  1 ] = _aoDirLightOrientationArrowVerts[ 46 ]; // B
	_aoDirLightOrientationArrowOutlineVerts[  2 ] = _aoDirLightOrientationArrowVerts[ 50 ]; // C
	_aoDirLightOrientationArrowOutlineVerts[  3 ] = _aoDirLightOrientationArrowVerts[ 56 ]; // D
	_aoDirLightOrientationArrowOutlineVerts[  4 ] = _aoDirLightOrientationArrowVerts[ 59 ]; // E
	_aoDirLightOrientationArrowOutlineVerts[  5 ] = _aoDirLightOrientationArrowVerts[ 21 ]; // F
	_aoDirLightOrientationArrowOutlineVerts[  6 ] = _aoDirLightOrientationArrowVerts[ 22 ]; // G
	_aoDirLightOrientationArrowOutlineVerts[  7 ] = _aoDirLightOrientationArrowVerts[ 26 ]; // A
	_aoDirLightOrientationArrowOutlineVerts[  8 ] = _aoDirLightOrientationArrowVerts[ 32 ]; // H
	_aoDirLightOrientationArrowOutlineVerts[  9 ] = _aoDirLightOrientationArrowVerts[ 38 ]; // I
	_aoDirLightOrientationArrowOutlineVerts[ 10 ] = _aoDirLightOrientationArrowVerts[ 46 ]; // B
	_aoDirLightOrientationArrowOutlineVerts[ 11 ] = _aoDirLightOrientationArrowVerts[ 38 ]; // I
	_aoDirLightOrientationArrowOutlineVerts[ 12 ] = _aoDirLightOrientationArrowVerts[ 51 ]; // J
	_aoDirLightOrientationArrowOutlineVerts[ 13 ] = _aoDirLightOrientationArrowVerts[ 50 ]; // C
	_aoDirLightOrientationArrowOutlineVerts[ 14 ] = _aoDirLightOrientationArrowVerts[ 51 ]; // J
	_aoDirLightOrientationArrowOutlineVerts[ 15 ] = _aoDirLightOrientationArrowVerts[ 53 ]; // K
	_aoDirLightOrientationArrowOutlineVerts[ 16 ] = _aoDirLightOrientationArrowVerts[ 56 ]; // D
	_aoDirLightOrientationArrowOutlineVerts[ 17 ] = _aoDirLightOrientationArrowVerts[ 53 ]; // K
	_aoDirLightOrientationArrowOutlineVerts[ 18 ] = _aoDirLightOrientationArrowVerts[ 12 ]; // L
	_aoDirLightOrientationArrowOutlineVerts[ 19 ] = _aoDirLightOrientationArrowVerts[ 59 ]; // E
	_aoDirLightOrientationArrowOutlineVerts[ 20 ] = _aoDirLightOrientationArrowVerts[ 12 ]; // L
	_aoDirLightOrientationArrowOutlineVerts[ 21 ] = _aoDirLightOrientationArrowVerts[ 36 ]; // M
	_aoDirLightOrientationArrowOutlineVerts[ 22 ] = _aoDirLightOrientationArrowVerts[ 21 ]; // F
	_aoDirLightOrientationArrowOutlineVerts[ 23 ] = _aoDirLightOrientationArrowVerts[ 36 ]; // M
	_aoDirLightOrientationArrowOutlineVerts[ 24 ] = _aoDirLightOrientationArrowVerts[ 33 ]; // N
	_aoDirLightOrientationArrowOutlineVerts[ 25 ] = _aoDirLightOrientationArrowVerts[ 22 ]; // G
	_aoDirLightOrientationArrowOutlineVerts[ 26 ] = _aoDirLightOrientationArrowVerts[ 33 ]; // N
	_aoDirLightOrientationArrowOutlineVerts[ 27 ] = _aoDirLightOrientationArrowVerts[ 32 ]; // H

	for( _uIndex = 0; _uIndex < 28; ++_uIndex )
	{
		_aoDirLightOrientationArrowOutlineVerts[ _uIndex ].ColorRGBA.Set( _DIRECTIONAL_LIGHT_ARROW_OUTLINE_R, _DIRECTIONAL_LIGHT_ARROW_OUTLINE_G, _DIRECTIONAL_LIGHT_ARROW_OUTLINE_B, _DIRECTIONAL_LIGHT_ARROW_OUTLINE_A );
	}
	//
	////

	_oBackgroundXfm.Identity();
	_nFilterPosition = _nFilterCharIndex = 0;
	_poAnimInst = NULL;
	_poAnimCombiner = NULL;
	_bShowMenu = TRUE;
	_poCurrentAnimation = NULL;
	_eCurrentMenu = _MENU_MAINMENU;
	_nResourceMenuSelection = 0;
	_uStartpoints = _uCurrentStartpoint = 0;

	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;
	FVis_bDrawVisibility = FALSE;
	FVis_bShowDrawStats = TRUE;
	
	return TRUE;

} // _ViewerInit

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ViewerTerm( FLoopTermCode_t nTermCode, void *pParameter )
{
	_SAFE_FMEM_RELEASEFRAME( _ohMemFrame );
	_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );

#ifdef _AUDIO_TEST
	faudio_Uninstall();
#endif

	ftext_Uninstall();
	fpadio_Uninstall();
	fpad_Uninstall();
	fstorage_Uninstall();
} // _ViewerTerm

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _ViewerMain( BOOL bExitRequest, void *pParameter )
{
	if( bExitRequest )
	{
		return FALSE;
	}

//	ViewerInitParm_t *pParm = (ViewerInitParm_t *)pParameter;

	////
	//
	_ManagePad();

	if( ! _uCurrentPadPlusOne )
	{
		_fPadErrorMessageDelay += FLoop_fRealPreviousLoopSecs;
		if( _PAD_ERROR_MESSAGE_DELAY < _fPadErrorMessageDelay )
		{
			if ( !fvid_Swap() )
			{
				return FALSE;
			}
			fvid_Begin();
			fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
			ftext_DebugPrintf( 0.5f, 0.35f, "~w1~ac~c59000099Please connect a controller" );
			fvid_End();
			return TRUE;
		}
	}
	else
	{
		_fPadErrorMessageDelay = 0.0f;
	}
	//
	////

	////
	//
	_ManageStorage();

	if( ! _uCurrentStorage )
	{
		if( ( 9 == _nMainMenuSelection ) || ( 10 == _nMainMenuSelection ) )
		{
			_nMainMenuSelection = 7;
		}
	}
	//
	////

	////
	//
	_ProcessPadStatesCommon();
	_ProcessPadStatesSubject();
	_ProcessPadStatesMenu();
	//
	////

	////
	//
	_ManageSubject();
	_ManageMenu();
	//
	////

	if ( !fvid_Swap() )
	{
		return FALSE;
	}

	////
	//
	fvid_Begin();
	if( _bShowMenu )
	{
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
	}
	else
	{
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, _oSettings.afColorBackgroundRGBI[ 0 ], _oSettings.afColorBackgroundRGBI[ 1 ], _oSettings.afColorBackgroundRGBI[ 2 ], 1.0f, 0 );
	}

	_DisplaySubject();

	if( _bShowMenu || _bShowHelp )
	{
		//// Gray-out background.
		//
		fviewport_SetActive( _poBackgroundViewport );
		_oBackgroundXfm.InitStackWithView();
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
		fdraw_SetTexture( NULL );
		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, _aoBackgroundVerts, 4 );
		frenderer_Pop();
		//
		////
	}

	_DisplayMenu();

/*
//test error..
fclib_strcpy( _szErrorMessage, "Couldn't load animation." );
_bShowError = TRUE;
_bPreviousShowError = FALSE;
*/

	_DisplayError();
	_DisplayHelp();

#if _AUDIO_TEST
#if _USAGE_STATS
	//// Usage statistics.
	//
	_oPerformaceTimer.Reset();
//	faudio_Work();
	_fMilisecondsTemp = 1000.0f * _oPerformaceTimer.SampleSeconds();

	++_uTimeStamp;
	_fUsageTotMiliseconds += _fMilisecondsTemp;

	if( _fMilisecondsTemp < _fUsageMinMiliseconds )
	{
		_fUsageMinMiliseconds = _fMilisecondsTemp;
	}

	if( _fMilisecondsTemp > _fUsageMaxMiliseconds )
	{
		_fUsageMaxMiliseconds = _fMilisecondsTemp;
	}
	//
	////

	//// Print usage statistics.
	//
	_fUsageReportDelay += FLoop_fRealPreviousLoopSecs;

	if( _USAGE_REPORT_DELAY <= _fUsageReportDelay )
	{
		_fUsageReportDelay = 0.0f;

		DEVPRINTF( "[ VIEWER ] Audio work: Min: %f Max: %f Avg: %f\n", _fUsageMinMiliseconds, _fUsageMaxMiliseconds, ( _fUsageTotMiliseconds / (f32)_uTimeStamp ) );

		_fUsageMinMiliseconds = 1000000000.0f;
		_fUsageMaxMiliseconds = 0.0f;
		_fUsageTotMiliseconds = 0.0f;
		_uTimeStamp = 0;
	}
	//
	////
#endif
#endif

	fvid_End();
	//
	////

	return TRUE;

} // _ViewerMain

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManagePad( void )
{
	_uPadConnected = fpad_UpdateSamples();
	if( _uPadConnected == _uPadConnectedPreviously )
	{
		return;
	}

	_uPadConnectedPreviously = _uPadConnected;

	if( ( _uCurrentPadPlusOne ) && ( _uPadConnected & ( 1 << ( _uCurrentPadPlusOne - 1 ) ) ) )
	{
		return;
	}

	_uCurrentPadPlusOne = 0;

	for( _uIndex = 0; _uIndex < FPADIO_MAX_DEVICES; ++_uIndex )
	{
		if( ! ( _uPadConnected & ( 1 << _uIndex ) ) )
		{
			continue;
		}

		_uCurrentPadPlusOne = _uIndex + 1;
		fpad_Map( _uIndex, _ACTION_COUNT, _aoeInputID, _apoSamples );
		break;
	}

} // _ManagePad

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _LoadSettings( void )
{
	// Return if Currently used storage device has changed status during this game loop.
	if( ( ! ( _uCurrentStorage & _uStorageConnected ) ) || ( _uCurrentStorage & ( _uStorageInserted | _uStorageRemoved ) ) )
	{
		return FALSE;
	}

	if( FSTORAGE_ERROR_NONE != fstorage_ReadProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME, 0, &_oSettings, sizeof( _oSettings ) ) )
	{
		fstorage_DeleteProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME );
		return FALSE;
	}

	if( FSTORAGE_ERROR_NONE != fstorage_ValidateProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME, TRUE ) )
	{
		fstorage_DeleteProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME );
		return FALSE;
	}

	return TRUE;

} // _LoadSettings

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _SaveSettings( void )
{
	// Return if Currently used storage device has changed status during this game loop.
	if( ( ! ( _uCurrentStorage & _uStorageConnected ) ) || ( _uCurrentStorage & ( _uStorageInserted | _uStorageRemoved ) ) )
	{
		return FALSE;
	}

	if( FSTORAGE_ERROR_NONE != fstorage_WriteProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME, 0, &_oSettings, sizeof( _oSettings ) ) )
	{
		fstorage_DeleteProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME );
		return FALSE;
	}

	if( FSTORAGE_ERROR_NONE != fstorage_ValidateProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME, TRUE ) )
	{
		fstorage_DeleteProfile( (FStorage_DeviceID_e)_uCurrentStorage, _PROFILE_NAME );
		return FALSE;
	}

	return TRUE;

} // _SaveSettings

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageStorage( void )
{
	_bRestoreStates = FALSE;

	fstorage_UpdateDeviceInfos( &_uStorageConnected, &_uStorageInserted, &_uStorageRemoved );

	// Return if nothing removed / inserted during this game loop.
	if( ! ( _uStorageInserted || _uStorageRemoved ) )
	{
		return;
	}

	// Return if Currently used storage device hasn't changed status during this game loop.
	if( ( _uCurrentStorage & _uStorageConnected ) && ( ! ( _uCurrentStorage & ( _uStorageInserted | _uStorageRemoved ) ) ) )
	{
		return;
	}

	_uCurrentStorage = 0;

	_Settings_t oTempSettings;

#if FANG_PLATFORM_XB
	for( _uIndex = 1; _uIndex < FSTORAGE_MAX_DEVICES; ++_uIndex )
	{
		if( ! ( _uStorageConnected & ( 1 << _uIndex ) ) )
		{
			continue;
		}

		fstorage_ValidateProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, FALSE, &_bValid );

		if( ( _bValid ) && ( FSTORAGE_ERROR_NONE == fstorage_ReadProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, 0, &oTempSettings, sizeof( oTempSettings ) ) ) )
		{
			_uCurrentStorage = ( 1 << _uIndex );
			_bRestoreStates = TRUE;
			break;
		}
	}

	if( ( ! _uCurrentStorage ) && ( _uStorageConnected & ( 1 << 0 ) ) )
	{
		fstorage_ValidateProfile( (FStorage_DeviceID_e)( 1 << 0 ), _PROFILE_NAME, FALSE, &_bValid );

		if( ( _bValid ) && ( FSTORAGE_ERROR_NONE == fstorage_ReadProfile( (FStorage_DeviceID_e)( 1 << 0 ), _PROFILE_NAME, 0, &oTempSettings, sizeof( oTempSettings ) ) ) )
		{
			_uCurrentStorage = ( 1 << 0 );
			_bRestoreStates = TRUE;
		}
	}

	if( ! _uCurrentStorage )
	{
		for( _uIndex = 1; _uIndex < FSTORAGE_MAX_DEVICES; ++_uIndex )
		{
			if( ! ( _uStorageConnected & ( 1 << _uIndex ) ) )
			{
				continue;
			}

			if( FSTORAGE_ERROR_NONE != fstorage_CreateProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, sizeof( _oSettings ) ) )
			{
				continue;
			}

			if( FSTORAGE_ERROR_NONE != fstorage_WriteProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, 0, &_oSettings, sizeof( _oSettings ) ) )
			{
				fstorage_DeleteProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME );
				continue;
			}

			if( FSTORAGE_ERROR_NONE != fstorage_ValidateProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, TRUE ) )
			{
				fstorage_DeleteProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME );
				continue;
			}

			_uCurrentStorage = ( 1 << _uIndex );
			break;
		}

		if( ( ! _uCurrentStorage ) &&
			( _uStorageConnected & ( 1 << 0 ) ) &&
			( FSTORAGE_ERROR_NONE == fstorage_CreateProfile( (FStorage_DeviceID_e)( 1 << 0 ), _PROFILE_NAME, sizeof( _oSettings ) ) ) )
		{
			if( ( FSTORAGE_ERROR_NONE == fstorage_WriteProfile( (FStorage_DeviceID_e)( 1 << 0 ), _PROFILE_NAME, 0, &_oSettings, sizeof( _oSettings ) ) ) &&
				( FSTORAGE_ERROR_NONE == fstorage_ValidateProfile( (FStorage_DeviceID_e)( 1 << 0 ), _PROFILE_NAME, TRUE ) ) )
			{
				_uCurrentStorage = ( 1 << 0 );
			}
			else
			{
				fstorage_DeleteProfile( (FStorage_DeviceID_e)( 1 << 0 ), _PROFILE_NAME );
			}
		}
	}
#else
	for( _uIndex = 0; _uIndex < FSTORAGE_MAX_DEVICES; ++_uIndex )
	{
		if( ! ( _uStorageConnected & ( 1 << _uIndex ) ) )
		{
			continue;
		}

		fstorage_ValidateProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, FALSE, &_bValid );

		if( ( _bValid ) && ( FSTORAGE_ERROR_NONE == fstorage_ReadProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, 0, &oTempSettings, sizeof( oTempSettings ) ) ) )
		{
			_uCurrentStorage = ( 1 << _uIndex );
			_bRestoreStates = TRUE;
			break;
		}
	}

	if( ! _uCurrentStorage )
	{
		for( _uIndex = 0; _uIndex < FSTORAGE_MAX_DEVICES; ++_uIndex )
		{
			if( ! ( _uStorageConnected & ( 1 << _uIndex ) ) )
			{
				continue;
			}

			if( FSTORAGE_ERROR_NONE != fstorage_CreateProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, sizeof( _oSettings ) ) )
			{
				continue;
			}

			if( FSTORAGE_ERROR_NONE != fstorage_WriteProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, 0, &_oSettings, sizeof( _oSettings ) ) )
			{
				fstorage_DeleteProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME );
				continue;
			}

			if( FSTORAGE_ERROR_NONE != fstorage_ValidateProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME, TRUE ) )
			{
				fstorage_DeleteProfile( (FStorage_DeviceID_e)( 1 << _uIndex ), _PROFILE_NAME );
				continue;
			}

			_uCurrentStorage = ( 1 << _uIndex );
			break;
		}
	}
#endif

	if( _bRestoreStates )
	{
		_oSettings = oTempSettings;
		_oSettings.oeResourceID = (_ResourceTypeID_e)( 0 );
		_oSettings.szResourceName[ 0 ] = 0;
		_oSettings.szAnimationName[ 0 ] = 0;
	}

} // _ManageStorage

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageSubject( void )
{
	if( _poCurrentAnimation )
	{
		_poAnimInst->DeltaTime( FLoop_fPreviousLoopSecs );
		_poAnimCombiner->ComputeMtxPalette();
	}

	////
	//
	if( _bShowMenu || _bShowHelp )
	{
		return;
	}

	if( _bBackPressed )
	{
		_bBackPressed = FALSE;
		_bShowError   = FALSE;
		_bShowMenu    = TRUE;
		return;
	}
	//
	////

	////
	//
	if( _RESOURCE_TYPE_WORLD == _oSettings.oeResourceID )
	{
		_ManageWorldSubject();
	}
	else if( _RESOURCE_TYPE_MESH == _oSettings.oeResourceID )
	{
		_ManageMeshSubject();
	}
	else if( _RESOURCE_TYPE_TEXTURE == _oSettings.oeResourceID )
	{
		_ManageTextureSubject();
	}
	//
	////

} // _ManageSubject

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageMenu( void )
{
	if( ( ! _bShowMenu ) || _bShowHelp )
	{
		return;
	}

	if( _MENU_MAINMENU == _eCurrentMenu )
	{
		if( _bBackPressed )
		{
			_bBackPressed = FALSE;
			_bShowMenu    = FALSE;
			_bShowError   = FALSE;
			return;
		}

		_ManageMainMenu();
	}
	else if( _MENU_RESOURCE_MASK & _eCurrentMenu )
	{
		if( _bBackPressed )
		{
			_bBackPressed = FALSE;
			_eCurrentMenu = _MENU_MAINMENU;
			_bShowError   = FALSE;
			return;
		}

		_ManageResourceMenu();
	}
	else if( _MENU_COLOR_MASK & _eCurrentMenu )
	{
		if( _bBackPressed )
		{
			_bBackPressed = FALSE;
			_eCurrentMenu = _MENU_MAINMENU;
			_bShowError   = FALSE;
			return;
		}

		_ManageColorMenu();
	}

} // _ManageMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _DisplaySubject( void )
{
	fmesh_ResetLightList();
	
	if( ! _oSettings.oeResourceID )
	{
		if( ! ( _bShowMenu || _bShowHelp ) )
		{
			ftext_DebugPrintString( 0.5f, 0.375f, "~w1~ac~c59000099Nothing to display" );
		}

		return;
	}

	////
	//
	if( ! ( _bShowMenu || _bShowHelp ) )
	{
		if( _RESOURCE_TYPE_WORLD == _oSettings.oeResourceID )
		{
			ftext_DebugPrintf( 0.075f, 0.075f, "~w1FPS: %3.0f", FLoop_fPreviousLoopOOSecs );
			ftext_DebugPrintf( 0.500f, 0.075f, "~w1~ac%s", _pszCurrentSubjectName );
			ftext_DebugPrintf( 0.925f, 0.075f, "~w1~arCamera:\n%.2f (X)\n%.2f (Y)\n%.2f (Z)", _oCameraXfm.m_MtxR.m_vPos.x, _oCameraXfm.m_MtxR.m_vPos.y, _oCameraXfm.m_MtxR.m_vPos.z );

#if _AUDIO_TEST
			CFXfm oXfm;
			static CFXfm oXfmPrevious;
			oXfm = _oCameraXfm;
			oXfm.Invert();
			if( oXfm.m_MtxF != oXfmPrevious.m_MtxF )
			{
				faudio_SetListenerOrientation( 0, &( oXfm ) );
			}
			oXfmPrevious = oXfm;
#endif
		}
		if( _RESOURCE_TYPE_MESH == _oSettings.oeResourceID )
		{
			ftext_DebugPrintf( 0.5f, 0.075f, "~w1~ac%s", _pszCurrentSubjectName );
			if( _poCurrentAnimation )
			{
				ftext_DebugPrintf( 0.075f, 0.65f, "~w1Playing:\n%s", _pszCurrentAnimationName );
			}

			if( _MOVEMENT_TYPE_MESH_MOVE == _eMovementType )
			{
				ftext_DebugPrintf( 0.925f, 0.075f, "~w1~arMoving\nMesh" );
			}
			if( _MOVEMENT_TYPE_DIR_LIGHT_ORIENTATION == _eMovementType )
			{
				ftext_DebugPrintf( 0.925f, 0.075f, "~w1~arMoving\nDir Light" );
			}
		}
		if( _RESOURCE_TYPE_TEXTURE == _oSettings.oeResourceID )
		{
			ftext_DebugPrintf( 0.500f, 0.075f, "~w1~ac%s", _pszCurrentSubjectName );
			ftext_DebugPrintf( 0.925f, 0.075f, "~w1~ar%u x %u", ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross, ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown );

//ARG - >>>>>
#if FANG_PLATFORM_PS2
			PS2EditFilter();
#endif
//ARG - <<<<<
		}
	}
	//
	////

	fviewport_SetActive( _poMainViewport );

	////
	//
	if( _RESOURCE_TYPE_WORLD == _oSettings.oeResourceID )
	{
		_oCameraXfm.InitStackWithView();

		fvis_FrameBegin();
		fvis_Draw( NULL, FALSE );
		fvis_FrameEnd();

#if _AUDIO_TEST
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_Depth_EnableWriting( TRUE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
		fdraw_SetTexture( NULL );

		_oArrowXfm.Identity();
		_oArrowXfm.PushModel();
		/*
		FDrawVtx_t oVtx;
		oVtx.ColorRGBA.OpaqueWhite();
		oVtx.Pos_MS.Zero();

		fdraw_Point( &oVtx );
		*/

		CFVec3 oVec( _AUDIO_POSITION_X, _AUDIO_POSITION_Y, _AUDIO_POSITION_Z );
		//oVec.Zero();

		CFColorRGBA oColor;
		oColor.OpaqueWhite();

		fdraw_FacetedWireSphere( &oVec, _AUDIO_RADIUS_OUTER, &oColor, 256 );
		fdraw_FacetedWireSphere( &oVec, 1.0f, &oColor, 256 );

		_oArrowXfm.PopModel();

		frenderer_Pop();
#endif
	}
	else if( _RESOURCE_TYPE_MESH == _oSettings.oeResourceID )
	{
		fmesh_Ambient_Set( _oSettings.afColorAmbientRGBI[ 0 ], _oSettings.afColorAmbientRGBI[ 1 ], _oSettings.afColorAmbientRGBI[ 2 ], 1.0f );

		_oDirectionalLight.InitDirLight( _oSettings.afWSDirectionalXYZ[ 0 ], _oSettings.afWSDirectionalXYZ[ 1 ], _oSettings.afWSDirectionalXYZ[ 2 ] );
		_oDirectionalLight.SetColor( _oSettings.afColorDirectionalRGBI[ 0 ], _oSettings.afColorDirectionalRGBI[ 1 ], _oSettings.afColorDirectionalRGBI[ 2 ], 1.0f );
		_oMeshInst.ResetLightList();
		_oMeshInst.ConsiderLightForRender( &_oDirectionalLight );

		_oCameraXfm.Identity();
		_oCameraXfm.InitStackWithView();
		_oMeshInst.m_Xfm = _oMeshXfm;
		frenderer_Push( FRENDERER_MESH );
		_oMeshInst.Draw( FVIEWPORT_PLANESMASK_ALL );
		frenderer_Pop();
/*
		if( _MOVEMENT_TYPE_DIR_LIGHT_ORIENTATION == _eMovementType )
		{
fviewport_SetActive( _poArrowViewport );
//fviewport_Clear( FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

CFXfm scale;
scale.BuildScale( 20.0f );

			_oArrowXfm.BuildRotYXZ_XlatFromPoint( _oSettings.afWSDirectionalHP[ 0 ], _oSettings.afWSDirectionalHP[ 1 ], 0.0f, _DIRECTIONAL_LIGHT_ARROW_X, _DIRECTIONAL_LIGHT_ARROW_Y, _DIRECTIONAL_LIGHT_ARROW_Z + 1000.0f );

			_oArrowXfm.PushModel();

scale.PushModel();

			frenderer_Push( FRENDERER_DRAW, NULL );

//			fdraw_Depth_EnableWriting( FALSE );
//			fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );

fdraw_Depth_EnableWriting( TRUE );
fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
			fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
			fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
			fdraw_SetTexture( NULL );

//fviewport_Clear( FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

			fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _aoDirLightOrientationArrowVerts, 60 );
			fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, _aoDirLightOrientationArrowOutlineVerts, 28 );
			frenderer_Pop();

scale.PopModel();
			_oArrowXfm.PopModel();

		}
*/
	}
	else if( _RESOURCE_TYPE_TEXTURE == _oSettings.oeResourceID )
	{
		_oCameraXfm.Identity();
		_oCameraXfm.InitStackWithView();
		_oMeshXfm.PushModel();
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
		fdraw_SetTexture( &_oTexInst );
		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, _aoTextureVerts, 4 );
		frenderer_Pop();
		_oMeshXfm.PopModel();
	}
	//
	////

} // _DisplaySubject

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _DisplayMenu( void )
{
	if( ( ! _bShowMenu ) || _bShowHelp )
	{
		return;
	}

	if( _MENU_MAINMENU == _eCurrentMenu )
	{
		_PrintMainMenu();
	}
	else if( _MENU_RESOURCE_MASK & _eCurrentMenu )
	{
		_PrintResourceMenu();
	}
	else if( _MENU_COLOR_MASK & _eCurrentMenu )
	{
		_PrintColorMenu();
	}

} // _DisplayMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _DisplayError( void )
{
	if( _bShowError )
	{
		static FSysinfo_DateTime_t __oDateTime;
		if( ! _bPreviousShowError )
		{
			__oDateTime = fsysinfo_CurrentDatetime();
			_bPreviousShowError = TRUE;
		}
		ftext_DebugPrintf( 0.5f, 0.625f, "~w1~ac~c59000099[ %2u/%2u/%4u - %2u:%02u:%02u %s ]\n%s", FSYSINFO_DATETIME_TO_MONTH( __oDateTime ), FSYSINFO_DATETIME_TO_DAYOFMONTH( __oDateTime ), ( 2000 + FSYSINFO_DATETIME_TO_YEARFROM2000( __oDateTime ) ), ( ( FSYSINFO_DATETIME_TO_HOUR( __oDateTime ) > 12 ) ? ( FSYSINFO_DATETIME_TO_HOUR( __oDateTime ) - 12 ) : FSYSINFO_DATETIME_TO_HOUR( __oDateTime ) ), FSYSINFO_DATETIME_TO_MIN( __oDateTime ), FSYSINFO_DATETIME_TO_SEC( __oDateTime ), ( ( FSYSINFO_DATETIME_TO_HOUR( __oDateTime ) > 12 ) ? "pm" : "am" ), _szErrorMessage );
	}
	else
	{
		_bPreviousShowError = FALSE;
	}

} // _DisplayError

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _DisplayHelp( void )
{
	if( _bShowHelp )
	{
		if( _bShowMenu )
		{
			if( _MENU_MAINMENU == _eCurrentMenu )
			{
				_PrintHelpMainMenu();
			}
			else if( _MENU_RESOURCE_MASK & _eCurrentMenu )
			{
				_PrintHelpResourceMenu();
			}
			else if( _MENU_COLOR_MASK & _eCurrentMenu )
			{
				_PrintHelpColorMenu();
			}
		}
		else
		{
			if( _RESOURCE_TYPE_WORLD == _oSettings.oeResourceID )
			{
				_PrintHelpWorldDisplay();
			}
			else if( _RESOURCE_TYPE_MESH == _oSettings.oeResourceID )
			{
				_PrintHelpMeshDisplay();
			}
			else if( _RESOURCE_TYPE_TEXTURE == _oSettings.oeResourceID )
			{
				_PrintHelpTextureDisplay();
			}
			else
			{
#if FANG_PLATFORM_GC
				ftext_DebugPrintf( 0.5f, 0.375f, "~w1~ac[X] Back" );
#else
				ftext_DebugPrintf( 0.5f, 0.375f, "~w1~ac[B] Back" );
#endif
			}
		}

		ftext_DebugPrintf( 0.5f, 0.675f, "~w1~ac[Y] Back" );
	}
	else
	{
		ftext_DebugPrintf( 0.5f, 0.675f, "~w1~ac[Y] help" );
	}

} // _DisplayHelp

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintMainMenu( void )
{
	static char __aszMainMenu[ _MAIN_MENU_NUM_ENTIRES ][ 31 ] =	{
																	"Select a world      (.wld)",     //  0
																	"Select a mesh       (.ape)",     //  1
																	"Select an animation (.mtx)",     //  2
																	"Select a texture    (.tga)",     //  3
																	"-",                              //  4
																	"Change background color   ",     //  5
																	"Change ambient color      ",     //  6
																	"Change directional color  ",     //  7
																	"-",                              //  8
																	"Load settings",                  //  9
																	"Save settings",                  // 10
																	"-",                              // 11
																	"Exit"                            // 12
																};

	sprintf( _szTempOutput, "~w1Main Menu :\n\n" );

	//-------------------------------------------------------------------------
	// 0. Select a World
	if( _RESOURCE_TYPE_WORLD == _oSettings.oeResourceID )
	{
		sprintf( _szTempLine, "%s%s%s%s%s%s\n", ( 0 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 0 ], ( 0 == _nMainMenuSelection ? "<~c59590099" : " " ), " (", _oSettings.szResourceName, ")" );
	}
	else
	{
		sprintf( _szTempLine, "%s%s%s\n", ( 0 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 0 ], ( 0 == _nMainMenuSelection ? "<~c59590099" : " " ) );
	}
	strcat( _szTempOutput, _szTempLine );

	//-------------------------------------------------------------------------
	// 1 & 2: Select a mesh, select an animation
	if( _RESOURCE_TYPE_MESH == _oSettings.oeResourceID )
	{
		// 1. Select a mesh
		sprintf( _szTempLine, "%s%s%s%s%s%s\n", ( 1 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 1 ], ( 1 == _nMainMenuSelection ? "<~c59590099" : " " ), "(", _oSettings.szResourceName, ")" );
		strcat( _szTempOutput, _szTempLine );

		// 2. Select an animation
		if( _poCurrentAnimation )
		{
			sprintf( _szTempLine, "%s%s%s%s%s%s\n", ( 2 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 2 ], ( 2 == _nMainMenuSelection ? "<~c59590099" : " " ), "(", _oSettings.szAnimationName, ")" );
		}
		else
		{
			sprintf( _szTempLine, "%s%s%s\n", ( 2 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 2 ], ( 2 == _nMainMenuSelection ? "<~c59590099" : " " ) );
		}
		strcat( _szTempOutput, _szTempLine );
	}
	else
	{
		// 1. Select a mesh
		sprintf( _szTempLine, "%s%s%s\n", ( 1 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 1 ], ( 1 == _nMainMenuSelection ? "<~c59590099" : " " ) );
		strcat( _szTempOutput, _szTempLine );

		// 2. Select an animation
		sprintf( _szTempLine, "~c39390099 %s~c59590099\n", __aszMainMenu[ 2 ] );
		strcat( _szTempOutput, _szTempLine );
	}

	//-------------------------------------------------------------------------
	// 3. Select a Texture
	if( _RESOURCE_TYPE_TEXTURE == _oSettings.oeResourceID )
	{
		sprintf( _szTempLine, "%s%s%s%s%s%s\n", ( 3 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 3 ], ( 3 == _nMainMenuSelection ? "<~c59590099" : " " ), "(", _oSettings.szResourceName, ")" );
	}
	else
	{
		sprintf( _szTempLine, "%s%s%s\n", ( 3 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 3 ], ( 3 == _nMainMenuSelection ? "<~c59590099" : " " ) );
	}
	strcat( _szTempOutput, _szTempLine );

	// 4. blank line
	strcat( _szTempOutput, "\n" );

	// 5. Change Background color
	sprintf( _szTempLine, "%s%s%s (%02u,%02u,%02u)\n", ( 5 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 5 ], ( 5 == _nMainMenuSelection ? "<~c59590099" : " " ), (u32)( _oSettings.afColorBackgroundRGBI[ 0 ] * 99.0f ), (u32)( _oSettings.afColorBackgroundRGBI[ 1 ] * 99.0f ), (u32)( _oSettings.afColorBackgroundRGBI[ 2 ] * 99.0f ) );
	strcat( _szTempOutput, _szTempLine );

	// 6. Change ambient color
	sprintf( _szTempLine, "%s%s%s (%02u,%02u,%02u)\n", ( 6 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 6 ], ( 6 == _nMainMenuSelection ? "<~c59590099" : " " ), (u32)( _oSettings.afColorAmbientRGBI[ 0 ] * 99.0f ), (u32)( _oSettings.afColorAmbientRGBI[ 1 ] * 99.0f ), (u32)( _oSettings.afColorAmbientRGBI[ 2 ] * 99.0f ) );
	strcat( _szTempOutput, _szTempLine );

	// 7. Change directional color
	sprintf( _szTempLine, "%s%s%s (%02u,%02u,%02u)\n", ( 7 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 7 ], ( 7 == _nMainMenuSelection ? "<~c59590099" : " " ), (u32)( _oSettings.afColorDirectionalRGBI[ 0 ] * 99.0f ), (u32)( _oSettings.afColorDirectionalRGBI[ 1 ] * 99.0f ), (u32)( _oSettings.afColorDirectionalRGBI[ 2 ] * 99.0f ) );
	strcat( _szTempOutput, _szTempLine );

	// 8. blank line
	strcat( _szTempOutput, "\n" );
// /*
	if( _uCurrentStorage )
	{
		// 9. load settings
		sprintf( _szTempLine, "%s%s%s", ( 9 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 9 ], ( 9 == _nMainMenuSelection ? "<~c59590099" : " " ) );
		strcat( _szTempOutput, _szTempLine );

#if FANG_PLATFORM_XB
		if( FSTORAGE_DEVICE_ID_XB_PC_HD == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Harddisk)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_0A == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 1 Slot A)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_0B == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 1 Slot B)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_1A == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 2 Slot A)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_1B == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 2 Slot B)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_2A == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 3 Slot A)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_2B == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 3 Slot B)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_3A == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 4 Slot A)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_XB_MC_3B == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Port 4 Slot B)\n" );
		}
#elif FANG_PLATFORM_GC
		if( FSTORAGE_DEVICE_ID_GC_MC_LEFT == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Memory Card A)\n" );
		}
		else if( FSTORAGE_DEVICE_ID_GC_MC_RIGHT == _uCurrentStorage )
		{
			sprintf( _szTempLine, " \\-> (Memory Card B)\n" );
		}
#elif FANG_PLATFORM_WIN
		sprintf( _szTempLine, " \\-> (Harddisk)\n" );
#endif
		strcat( _szTempOutput, _szTempLine );

		// 10. save settings
		sprintf( _szTempLine, "%s%s%s /\n", ( 10 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 10 ], ( 10 == _nMainMenuSelection ? "<~c59590099" : " " ) );
		strcat( _szTempOutput, _szTempLine );
	}
	else
	{
		// 9. load settings
		sprintf( _szTempLine, " ~c39390099%s  \\-> (No device connected)\n", __aszMainMenu[ 9 ] );
		strcat( _szTempOutput, _szTempLine );

		// 10. save settings
		sprintf( _szTempLine, " %s  /~c59590099\n", __aszMainMenu[ 10 ] );
		strcat( _szTempOutput, _szTempLine );
	}

	// 11. blank line
	strcat( _szTempOutput, "\n" );
// */
	// 12. Exit
	sprintf( _szTempLine, "%s%s%s\n", ( 12 == _nMainMenuSelection ? "~c79797999>" : " " ), __aszMainMenu[ 12 ], ( 12 == _nMainMenuSelection ? "<~c59590099" : "" ) );
	strcat( _szTempOutput, _szTempLine );

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

	ftext_DebugPrintf( 0.925f, 0.675f, "~w1~arVer: %u.%u.%u", fversion_GetToolMajorVer(), 
															  fversion_GetToolMinorVer(),
															  fversion_GetToolSubVer() );

} // _PrintMainMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintResourceMenu( void )
{
	if( _MENU_SELECT_WORLD == _eCurrentMenu )
	{
		sprintf( _szTempOutput, "~w1Main Menu :\nSelect a world (.wld) :" );
	}
	else if( _MENU_SELECT_MESH == _eCurrentMenu )
	{
		sprintf( _szTempOutput, "~w1Main Menu :\nSelect a mesh (.ape) :" );
	}
	else if( _MENU_SELECT_ANIMATION == _eCurrentMenu )
	{
		sprintf( _szTempOutput, "~w1Main Menu :\nSelect an animation (.mtx) :" );
	}
	else if( _MENU_SELECT_TEXTURE == _eCurrentMenu )
	{
		sprintf( _szTempOutput, "~w1Main Menu :\nSelect a texture (.tga) :" );
	}

	sprintf( _szTempLine, "\n\n           Filter: ~c39395999" );
	strcat( _szTempOutput, _szTempLine );

	for( _uIndex = 0; _uIndex < _RESOURCE_NAME_LENGTH; ++_uIndex )
	{
		if( _uIndex == _nFilterPosition )
		{
			sprintf( _szTempLine, "~ba%c~b0", _pszCurrentFilter[ _uIndex ] );
			strcat( _szTempOutput, _szTempLine );
		}
		else
		{
			sprintf( _szTempLine, "%c", _pszCurrentFilter[ _uIndex ] );
			strcat( _szTempOutput, _szTempLine );
		}
	}

	sprintf( _szTempLine, "~c59590099\n\n" );
	strcat( _szTempOutput, _szTempLine );

	static u32 __uLastIndexOnCurrentPage, __uCount, __uIndex;
	_pszTempString = &( _papszFiltered[ _uFirstIndexOnCurrentPage ] );
	__uLastIndexOnCurrentPage = FMATH_MIN( ( _uFirstIndexOnCurrentPage + ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ), _uFiltered );

	static BOOL __bColorChanged;
	__bColorChanged = FALSE;

	for( _uIndex = 0; _uIndex < _MENU_LINES_PER_COLS; ++_uIndex )
	{
		strcat( _szTempOutput, " " );

		for( _uTemp = 0; _uTemp < _MENU_COLS_PER_WINDOW; ++_uTemp )
		{
			__uCount = ( _uIndex + ( _uTemp * _MENU_LINES_PER_COLS ) );
			if( ( _uFirstIndexOnCurrentPage + __uCount ) < __uLastIndexOnCurrentPage )
			{
				if( ( _uFirstIndexOnCurrentPage + __uCount ) == _nResourceMenuSelection )
				{
					sprintf( _szTempLine, "~c79797999>%-11s<~c59590099", _pszTempString[ __uCount ] );
					strcat( _szTempOutput, _szTempLine );
				}
				else
				{
					strcat( _szTempOutput, " " );

					for( __uIndex = 0; __uIndex < _RESOURCE_NAME_LENGTH; ++__uIndex )
					{
						if( ! _pszTempString[ __uCount ][ __uIndex ] )
						{
							strcat( _szTempOutput, " " );
							continue;
						}

						if( _pszTempString[ __uCount ][ __uIndex ] == _pszCurrentFilter[ __uIndex ] )
						{
							if( __bColorChanged )
							{
								sprintf( _szTempLine, "%c", _pszTempString[ __uCount ][ __uIndex ] );
							}
							else
							{
								sprintf( _szTempLine, "~c39395999%c", _pszTempString[ __uCount ][ __uIndex ] );
								__bColorChanged = TRUE;
							}
						}
						else
						{
							if( __bColorChanged )
							{
								sprintf( _szTempLine, "~c59590099%c", _pszTempString[ __uCount ][ __uIndex ] );
								__bColorChanged = FALSE;
							}
							else
							{
								sprintf( _szTempLine, "%c", _pszTempString[ __uCount ][ __uIndex ] );
							}
						}

						strcat( _szTempOutput, _szTempLine );
					}

					strcat( _szTempOutput, " " );
				}
			}
		}

		strcat( _szTempOutput, "\n" );
	}

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

	ftext_DebugPrintf( 0.925f, 0.075f, "~w1~arColumn: %u of %u\n%u Elements", ( 1 + ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) ), ( 1 + ( _uFiltered / _MENU_LINES_PER_COLS ) ), _uFiltered );

} // _PrintResourceMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintColorMenu( void )
{
	if( _MENU_SET_BACKGROUND == _eCurrentMenu )
	{
		sprintf( _szTempOutput, "~w1Main Menu :\nChange background color :\n\n\n\n" );
	}
	else if( _MENU_SET_AMBIENT == _eCurrentMenu )
	{
		sprintf( _szTempOutput, "~w1Main Menu :\nChange ambient color :\n\n\n\n" );
	}
	else if( _MENU_SET_DIRECTIONAL == _eCurrentMenu )
	{
		sprintf( _szTempOutput, "~w1Main Menu :\nChange directional color :\n\n\n\n" );
	}

	sprintf( _szTempLine, "~w1\t\t~b%cR~b0: %02u~w0 --------------------------------------------------\n", ( ( 0 == _nColorMenuSelection ) ? 'a' : '0' ), (u32)( _pafCurrentColor[ 0 ] * 99.0f ) );
	_szTempLine[ 20 + (u32)( _pafCurrentColor[ 0 ] * 49.5f ) ] = '+';
	strcat( _szTempOutput, _szTempLine );

	sprintf( _szTempLine, "~w1\t\t~b%cG~b0: %02u~w0 --------------------------------------------------\n", ( ( 1 == _nColorMenuSelection ) ? 'a' : '0' ), (u32)( _pafCurrentColor[ 1 ] * 99.0f ) );
	_szTempLine[ 20 + (u32)( _pafCurrentColor[ 1 ] * 49.5f ) ] = '+';
	strcat( _szTempOutput, _szTempLine );

	sprintf( _szTempLine, "~w1\t\t~b%cB~b0: %02u~w0 --------------------------------------------------\n", ( ( 2 == _nColorMenuSelection ) ? 'a' : '0' ), (u32)( _pafCurrentColor[ 2 ] * 99.0f ) );
	_szTempLine[ 20 + (u32)( _pafCurrentColor[ 2 ] * 49.5f ) ] = '+';
	strcat( _szTempOutput, _szTempLine );

	sprintf( _szTempLine, "~w1\t\t~b%cI~b0: %02u~w0 --------------------------------------------------\n", ( ( 3 == _nColorMenuSelection ) ? 'a' : '0' ), (u32)( _pafCurrentColor[ 3 ] * 99.0f ) );
	_szTempLine[ 20 + (u32)( _pafCurrentColor[ 3 ] * 49.5f ) ] = '+';
	strcat( _szTempOutput, _szTempLine );

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

	//// Color sample box.
	//
	_aoColorSampleVerts[ 0 ].ColorRGBA.Set( _pafCurrentColor[ 0 ], _pafCurrentColor[ 1 ], _pafCurrentColor[ 2 ], 1.0f );
	_aoColorSampleVerts[ 1 ].ColorRGBA.Set( _pafCurrentColor[ 0 ], _pafCurrentColor[ 1 ], _pafCurrentColor[ 2 ], 1.0f );
	_aoColorSampleVerts[ 2 ].ColorRGBA.Set( _pafCurrentColor[ 0 ], _pafCurrentColor[ 1 ], _pafCurrentColor[ 2 ], 1.0f );
	_aoColorSampleVerts[ 3 ].ColorRGBA.Set( _pafCurrentColor[ 0 ], _pafCurrentColor[ 1 ], _pafCurrentColor[ 2 ], 1.0f );

	fviewport_SetActive( _poBackgroundViewport );
	_oBackgroundXfm.InitStackWithView();
	frenderer_Push( FRENDERER_DRAW, NULL );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_SetTexture( NULL );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, _aoColorSampleVerts, 4 );
	frenderer_Pop();
	//
	////

} // _PrintColorMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ProcessPadStatesCommon( void )
{
	if( ! _uCurrentPadPlusOne )
	{
		_bBackPressed = FALSE;
		return;
	}

	////
	//
	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_TOGGLE_MAINMENU ]->uLatches )
	{
		_bShowMenu = ! _bShowMenu;
		if( _bShowMenu )
		{
			_eCurrentMenu = _MENU_MAINMENU;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_BACK_TO_PREVIOUS ]->uLatches )
	{
		_bBackPressed = TRUE;
	}
	else
	{
		_bBackPressed = FALSE;
	}

	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_TOGGLE_HELP ]->uLatches )
	{
		_bShowHelp = ! _bShowHelp;
	}

	if( _bShowHelp && _bBackPressed )
	{
		_bShowHelp = FALSE;
		_bBackPressed = FALSE;
	}
	//
	////

} // _ProcessPadStatesCommon

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ProcessPadStatesSubject( void )
{
	fang_MemZero( &_oSubjectNavigation, sizeof( _oSubjectNavigation ) );

	if( ! _uCurrentPadPlusOne )
	{
		return;
	}

	////
	//
	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_CYCLE_VIEW ]->uLatches )
	{
		_oSubjectNavigation.bCycleView = TRUE;
	}

	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_CYCLE_MOVEMENT ]->uLatches )
	{
		_oSubjectNavigation.bCycleMove = TRUE;
	}
	//
	////

	////
	//
	_oSubjectNavigation.fX = _apoSamples[ _ACTION_PAN_LEFT_RIGHT1 ]->fCurrentState + _apoSamples[ _ACTION_PAN_LEFT_RIGHT2 ]->fCurrentState;
	FMATH_CLAMP( _oSubjectNavigation.fX, -1.0f, 1.0f );
	if( 0.0f < _oSubjectNavigation.fX )
	{
		_oSubjectNavigation.fX = ( _oSubjectNavigation.fX * _oSubjectNavigation.fX );
	}
	else
	{
		_oSubjectNavigation.fX = - ( _oSubjectNavigation.fX * _oSubjectNavigation.fX );
	}
	//
	////

	////
	//
	_oSubjectNavigation.fY = _apoSamples[ _ACTION_HOVER_UP ]->fCurrentState - _apoSamples[ _ACTION_HOVER_DOWN ]->fCurrentState;
	FMATH_CLAMP( _oSubjectNavigation.fY, -1.0f, 1.0f );
	if( 0.0f < _oSubjectNavigation.fY )
	{
		_oSubjectNavigation.fY = ( _oSubjectNavigation.fY * _oSubjectNavigation.fY );
	}
	else
	{
		_oSubjectNavigation.fY = - ( _oSubjectNavigation.fY * _oSubjectNavigation.fY );
	}
	//
	////

	////
	//
	_oSubjectNavigation.fZ = _apoSamples[ _ACTION_MOVE_FORWARD_BACKWARD1 ]->fCurrentState + _apoSamples[ _ACTION_MOVE_FORWARD_BACKWARD2 ]->fCurrentState;
	FMATH_CLAMP( _oSubjectNavigation.fZ, -1.0f, 1.0f );
	if( 0.0f < _oSubjectNavigation.fZ )
	{
		_oSubjectNavigation.fZ = ( _oSubjectNavigation.fZ * _oSubjectNavigation.fZ );
	}
	else
	{
		_oSubjectNavigation.fZ = - ( _oSubjectNavigation.fZ * _oSubjectNavigation.fZ );
	}
	//
	////

	////
	//
	_oSubjectNavigation.fHeading = _apoSamples[ _ACTION_HEADING_LEFT_RIGHT ]->fCurrentState;
	if( 0.0f < _oSubjectNavigation.fHeading )
	{
		_oSubjectNavigation.fHeading = ( _oSubjectNavigation.fHeading * _oSubjectNavigation.fHeading );
	}
	else
	{
		_oSubjectNavigation.fHeading = - ( _oSubjectNavigation.fHeading * _oSubjectNavigation.fHeading );
	}
	//
	////

	////
	//
#if _AUDIO_TEST
#if 0
	f32 fTemp = _apoSamples[ _ACTION_PITCH_UP_DOWN ]->fCurrentState;

	if( 0.0f < fTemp )
	{
		FMATH_CLAMP( fTemp, 0.0f, 1.0f );
		_poAudioEmitter->SetVolume( fTemp );
	}
	else
	{
		FMATH_CLAMP( fTemp, -1.0f, 0.0f );
		_poAudioEmitter->SetVolume( - fTemp );
	}
#endif
#endif

	_oSubjectNavigation.fPitch   = _apoSamples[ _ACTION_PITCH_UP_DOWN ]->fCurrentState;
	if( 0.0f < _oSubjectNavigation.fPitch )
	{
		_oSubjectNavigation.fPitch = ( _oSubjectNavigation.fPitch * _oSubjectNavigation.fPitch );
	}
	else
	{
		_oSubjectNavigation.fPitch = - ( _oSubjectNavigation.fPitch * _oSubjectNavigation.fPitch );
	}
	//
	////

} // _ProcessPadStatesSubject

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ProcessPadStatesMenu( void )
{
	fang_MemZero( &_oMenuNavigation, sizeof( _oMenuNavigation ) );

	if( ! _uCurrentPadPlusOne )
	{
		return;
	}

#if _AUDIO_TEST
	static int uWaveIndex;
	if( 0.7f < _apoSamples[ _ACTION_HOVER_DOWN ]->fCurrentState ) // LEFT TRIGGER
	{
		// TOP
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_TOGGLE_HELP ]->uLatches )
		{
			CFAudioEmitter::Play2D( _aohWaveHandles[ uWaveIndex ] );
			++uWaveIndex;
			uWaveIndex %= 4;
		}

		// BOTTOM
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_USE_SELECTION ]->uLatches )
		{
			CFAudioEmitter::Play2D( _aohWaveHandles[ 6 ] );
		}

		// RIGHT
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_BACK_TO_PREVIOUS ]->uLatches )
		{
			CFAudioEmitter::Play2D( _aohWaveHandles[ 5 ] );
		}

		// LEFT
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_SELECT_RESET_FILTER ]->uLatches )
		{
			CFAudioEmitter::Play2D( _aohWaveHandles[ 4 ] );
		}
	}
	else if( 0.7f < _apoSamples[ _ACTION_HOVER_UP ]->fCurrentState ) // RIGHT TRIGGER
	{
		// TOP
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_TOGGLE_HELP ]->uLatches )
		{
			if( FAUDIO_STREAM_STATE_STOPPED == _apoAudioStream[ 0 ]->GetState() )
			{
				_apoAudioStream[ 0 ]->SetVolume( 1.0f );
				_apoAudioStream[ 0 ]->SetPan( 1.0f );
				_apoAudioStream[ 0 ]->Play( 0 );
			}
			else if( FAUDIO_STREAM_STATE_PLAYING == _apoAudioStream[ 0 ]->GetState() )
			{
				_apoAudioStream[ 0 ]->Pause( TRUE );
			}
			else if( FAUDIO_STREAM_STATE_PAUSED == _apoAudioStream[ 0 ]->GetState() )
			{
				_apoAudioStream[ 0 ]->Pause( FALSE );
			}
		}

		// BOTTOM
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_USE_SELECTION ]->uLatches )
		{
			if( FAUDIO_STREAM_STATE_STOPPED == _apoAudioStream[ 1 ]->GetState() )
			{
				_apoAudioStream[ 1 ]->SetVolume( 1.0f );
				_apoAudioStream[ 1 ]->SetPan( -1.0f );
				_apoAudioStream[ 1 ]->Play( 0 );
			}
			else if( FAUDIO_STREAM_STATE_PLAYING == _apoAudioStream[ 1 ]->GetState() )
			{
				_apoAudioStream[ 1 ]->Pause( TRUE );
			}
			else if( FAUDIO_STREAM_STATE_PAUSED == _apoAudioStream[ 1 ]->GetState() )
			{
				_apoAudioStream[ 1 ]->Pause( FALSE );
			}
		}

		// RIGHT
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_BACK_TO_PREVIOUS ]->uLatches )
		{
			if( FAUDIO_STREAM_STATE_STOPPED == _apoAudioStream[ 2 ]->GetState() )
			{
				_apoAudioStream[ 2 ]->SetVolume( 1.0f );
				_apoAudioStream[ 2 ]->SetPan( 1.0f );
				_apoAudioStream[ 2 ]->Play( 0 );
			}
			else if( FAUDIO_STREAM_STATE_PLAYING == _apoAudioStream[ 2 ]->GetState() )
			{
				_apoAudioStream[ 2 ]->Pause( TRUE );
			}
			else if( FAUDIO_STREAM_STATE_PAUSED == _apoAudioStream[ 2 ]->GetState() )
			{
				_apoAudioStream[ 2 ]->Pause( FALSE );
			}
		}

		// LEFT
		if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_SELECT_RESET_FILTER ]->uLatches )
		{
			if( FAUDIO_STREAM_STATE_STOPPED == _apoAudioStream[ 3 ]->GetState() )
			{
				_apoAudioStream[ 3 ]->SetVolume( 1.0f );
				_apoAudioStream[ 3 ]->SetPan( -1.0f );
				_apoAudioStream[ 3 ]->Play( 0 );
			}
			else if( FAUDIO_STREAM_STATE_PLAYING == _apoAudioStream[ 3 ]->GetState() )
			{
				_apoAudioStream[ 3 ]->Pause( TRUE );
			}
			else if( FAUDIO_STREAM_STATE_PAUSED == _apoAudioStream[ 3 ]->GetState() )
			{
				_apoAudioStream[ 3 ]->Pause( FALSE );
			}
		}
	}
#endif

	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_USE_SELECTION ]->uLatches )
	{
		_oMenuNavigation.bSelect = TRUE;
	}

#define __THREASHOLD_PAD_DIRECTION		( 0.5f )

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_UP_DOWN1 ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_UP_DOWN1 ]->fCurrentState )
		{
			_oMenuNavigation.bUp = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_UP_DOWN1 ]->fCurrentState )
		{
			_oMenuNavigation.bDown = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_UP_DOWN2 ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_UP_DOWN2 ]->fCurrentState )
		{
			_oMenuNavigation.bUp = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_UP_DOWN2 ]->fCurrentState )
		{
			_oMenuNavigation.bDown = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_UP_DOWN3 ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_UP_DOWN3 ]->fCurrentState )
		{
			_oMenuNavigation.bAlternateUp = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_UP_DOWN3 ]->fCurrentState )
		{
			_oMenuNavigation.bAlternateDown = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_LEFT_RIGHT1 ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_LEFT_RIGHT1 ]->fCurrentState )
		{
			_oMenuNavigation.bRight = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_LEFT_RIGHT1 ]->fCurrentState )
		{
			_oMenuNavigation.bLeft = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_LEFT_RIGHT2 ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_LEFT_RIGHT2 ]->fCurrentState )
		{
			_oMenuNavigation.bRight = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_LEFT_RIGHT2 ]->fCurrentState )
		{
			_oMenuNavigation.bLeft = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_LEFT_RIGHT3 ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_LEFT_RIGHT3 ]->fCurrentState )
		{
			_oMenuNavigation.bAlternateRight = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_LEFT_RIGHT3 ]->fCurrentState )
		{
			_oMenuNavigation.bAlternateLeft = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_FILTER_UP_DOWN ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_FILTER_UP_DOWN ]->fCurrentState )
		{
			_oMenuNavigation.bFilterUp = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_FILTER_UP_DOWN ]->fCurrentState )
		{
			_oMenuNavigation.bFilterDown = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY & _apoSamples[ _ACTION_SELECT_FILTER_LEFT_RIGHT ]->uLatches )
	{
		if( __THREASHOLD_PAD_DIRECTION < _apoSamples[ _ACTION_SELECT_FILTER_LEFT_RIGHT ]->fCurrentState )
		{
			_oMenuNavigation.bFilterRight = TRUE;
		}
		else if( ( - __THREASHOLD_PAD_DIRECTION ) > _apoSamples[ _ACTION_SELECT_FILTER_LEFT_RIGHT ]->fCurrentState )
		{
			_oMenuNavigation.bFilterLeft = TRUE;
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_SELECT_RESET_FILTER ]->uLatches )
	{
		_oMenuNavigation.bFilterErase = TRUE;
	}

	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_SELECT_RESET_FILTER_LETTER ]->uLatches )
	{
		_oMenuNavigation.bFilterEraseLetter = TRUE;
	}
	//
	////

} // _ProcessPadStatesMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageMainMenu( void )
{
	if( _oMenuNavigation.bUp || _oMenuNavigation.bAlternateUp )
	{
		--_nMainMenuSelection;
		if( 0 > _nMainMenuSelection )
		{
			_nMainMenuSelection = _MAIN_MENU_NUM_ENTIRES - 1;
		}

		if( 11 == _nMainMenuSelection )
		{
			--_nMainMenuSelection;
		}
if( 10 == _nMainMenuSelection )
{
	--_nMainMenuSelection;
}
if(  9 == _nMainMenuSelection )
{
	--_nMainMenuSelection;
}
		if(  8 == _nMainMenuSelection )
		{
			--_nMainMenuSelection;
		}
		if(  4 == _nMainMenuSelection )
		{
			--_nMainMenuSelection;
		}

		if( ! _uCurrentStorage )
		{
			if( ( 9 == _nMainMenuSelection ) || ( 10 == _nMainMenuSelection ) )
			{
				_nMainMenuSelection = 7;
			}
		}

		if( _RESOURCE_TYPE_MESH != _oSettings.oeResourceID )
		{
			if( 2 == _nMainMenuSelection )
			{
				_nMainMenuSelection = 1;
			}
		}
	}
	else if( _oMenuNavigation.bDown || _oMenuNavigation.bAlternateDown )
	{
		++_nMainMenuSelection;
		if( ( _MAIN_MENU_NUM_ENTIRES - 1 ) < _nMainMenuSelection )
		{
			_nMainMenuSelection = 0;
		}

		if(  4 == _nMainMenuSelection )
		{
			++_nMainMenuSelection;
		}
		if(  8 == _nMainMenuSelection )
		{
			++_nMainMenuSelection;
		}
//if(  9 == _nMainMenuSelection )
//{
//	++_nMainMenuSelection;
//}
//if( 10 == _nMainMenuSelection )
//{
//	++_nMainMenuSelection;
//}
		if( 11 == _nMainMenuSelection )
		{
			++_nMainMenuSelection;
		}

		if( ! _uCurrentStorage )
		{
			if( ( 9 == _nMainMenuSelection ) || ( 10 == _nMainMenuSelection ) )
			{
				_nMainMenuSelection = 12;
			}
		}

		if( _RESOURCE_TYPE_MESH != _oSettings.oeResourceID )
		{
			if( 2 == _nMainMenuSelection )
			{
				_nMainMenuSelection = 3;
			}
		}
	}
	else if( _oMenuNavigation.bSelect )
	{
		switch( _nMainMenuSelection )
		{
		case 0:
				if( _uWorlds )
				{
					_eCurrentMenu = _MENU_SELECT_WORLD;

					_pszCurrentFilter = _oSettings.szWorldFilter;
					_pszCurrentSelection = _oSettings.szWorldSelection;
					_paszCurrentResourceList = _papszWorlds;
					_uCurrentResourceListLength = _uWorlds;

					_nFilterPosition = 0;
					_nFilterCharIndex = ( strchr( _szFilterChars, _pszCurrentFilter[ _nFilterPosition ] ) - _szFilterChars );

					_bShowError = FALSE;

					_BuildFilteredList();
					_FindCurrentSelectionIndex();
				}
				else
				{
					fclib_strcpy( _szErrorMessage, "No worlds available." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
				}
				break;
		case 1:
				if( _uMeshes )
				{
					_eCurrentMenu = _MENU_SELECT_MESH;

					_pszCurrentFilter = _oSettings.szMeshFilter;
					_pszCurrentSelection = _oSettings.szMeshSelection;
					_paszCurrentResourceList = _papszMeshes;
					_uCurrentResourceListLength = _uMeshes;

					_nFilterPosition = 0;
					_nFilterCharIndex = ( strchr( _szFilterChars, _pszCurrentFilter[ _nFilterPosition ] ) - _szFilterChars );

					_bShowError = FALSE;

					_BuildFilteredList();
					_FindCurrentSelectionIndex();
				}
				else
				{
					fclib_strcpy( _szErrorMessage, "No meshes available." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
				}
				break;
		case 2:
				if( _uAnimations )
				{
					_eCurrentMenu = _MENU_SELECT_ANIMATION;

					_pszCurrentFilter = _oSettings.szAnimationFilter;
					_pszCurrentSelection = _oSettings.szAnimationSelection;
					_paszCurrentResourceList = _papszAnimations;
					_uCurrentResourceListLength = _uAnimations;

					_nFilterPosition = 0;
					_nFilterCharIndex = ( strchr( _szFilterChars, _pszCurrentFilter[ _nFilterPosition ] ) - _szFilterChars );

					_bShowError = FALSE;

					_BuildFilteredList();
					_FindCurrentSelectionIndex();
				}
				else
				{
					fclib_strcpy( _szErrorMessage, "No animations available." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
				}
				break;
		case 3:
				if( _uTextures )
				{
					_eCurrentMenu = _MENU_SELECT_TEXTURE;

					_pszCurrentFilter = _oSettings.szTextureFilter;
					_pszCurrentSelection = _oSettings.szTextureSelection;
					_paszCurrentResourceList = _papszTextures;
					_uCurrentResourceListLength = _uTextures;

					_nFilterPosition = 0;
					_nFilterCharIndex = ( strchr( _szFilterChars, _pszCurrentFilter[ _nFilterPosition ] ) - _szFilterChars );

					_bShowError = FALSE;

					_BuildFilteredList();
					_FindCurrentSelectionIndex();
				}
				else
				{
					fclib_strcpy( _szErrorMessage, "No textures available." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
				}
				break;
		case 5:
				_eCurrentMenu = _MENU_SET_BACKGROUND;
				_nColorMenuSelection = 0;
				_bShowError = FALSE;
				_pafCurrentColor = _oSettings.afColorBackgroundRGBI;
				break;
		case 6:
				_eCurrentMenu = _MENU_SET_AMBIENT;
				_nColorMenuSelection = 0;
				_bShowError = FALSE;
				_pafCurrentColor = _oSettings.afColorAmbientRGBI;
				break;
		case 7:
				_eCurrentMenu = _MENU_SET_DIRECTIONAL;
				_nColorMenuSelection = 0;
				_bShowError = FALSE;
				_pafCurrentColor = _oSettings.afColorDirectionalRGBI;
				break;
		case 9:
// /*
				if( _LoadSettings() )
				{
					fclib_strcpy( _szErrorMessage, "Settings loaded." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
				}
				else
				{
// */
					fclib_strcpy( _szErrorMessage, "Error loading settings." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
// /*
				}
// */
				break;
		case 10:
// /*
				if( _SaveSettings() )
				{
					fclib_strcpy( _szErrorMessage, "Settings saved." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
				}
				else
				{
// */
					fclib_strcpy( _szErrorMessage, "Error saving settings." );
					_bShowError = TRUE;
					_bPreviousShowError = FALSE;
// /*
				}
// */
				break;
		case 12:
				_SaveSettings();
				fang_ResetHardware();
				break;
		}
	}

} // _ManageMainMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageResourceMenu( void )
{
	////
	//
	if( _oMenuNavigation.bFilterLeft )
	{
		--_nFilterPosition;
		if( 0 > _nFilterPosition )
		{
			_nFilterPosition = ( _RESOURCE_NAME_LENGTH - 1 );
		}
		_nFilterCharIndex = ( strchr( _szFilterChars, _pszCurrentFilter[ _nFilterPosition ] ) - _szFilterChars );
	}
	else if( _oMenuNavigation.bFilterRight )
	{
		++_nFilterPosition;
		if( ( _RESOURCE_NAME_LENGTH - 1 ) < _nFilterPosition )
		{
			_nFilterPosition = 0;
		}
		_nFilterCharIndex = ( strchr( _szFilterChars, _pszCurrentFilter[ _nFilterPosition ] ) - _szFilterChars );
	}
	else if( _oMenuNavigation.bFilterUp )
	{
		++( _nFilterCharIndex );
		if( ( _FILTER_NUM_CHARS_IN_SET - 1 ) < _nFilterCharIndex )
		{
			_nFilterCharIndex = 0;
		}
		_pszCurrentFilter[ _nFilterPosition ] = _szFilterChars[ _nFilterCharIndex ];
		_BuildFilteredList();
		_FindCurrentSelectionIndex();
	}
	else if( _oMenuNavigation.bFilterDown )
	{
		--( _nFilterCharIndex );
		if( 0 > _nFilterCharIndex )
		{
			_nFilterCharIndex = ( _FILTER_NUM_CHARS_IN_SET - 1 );
		}
		_pszCurrentFilter[ _nFilterPosition ] = _szFilterChars[ _nFilterCharIndex ];
		_BuildFilteredList();
		_FindCurrentSelectionIndex();
	}
	else if( _oMenuNavigation.bFilterErase )
	{
		fang_MemSet( _pszCurrentFilter, _FILTER_ANY_CHAR, _RESOURCE_NAME_LENGTH );
		_nFilterCharIndex = 0;
		_BuildFilteredList();
		_FindCurrentSelectionIndex();
	}
	else if( _oMenuNavigation.bFilterEraseLetter )
	{
		_pszCurrentFilter[ _nFilterPosition ] = _FILTER_ANY_CHAR;
		_nFilterCharIndex = 0;
		_BuildFilteredList();
		_FindCurrentSelectionIndex();
	}
	//
	////

	if( 0 == _uFiltered )
	{
		fang_MemZero( _pszCurrentSelection, _RESOURCE_NAME_LENGTH );
		return;
	}

	////
	//
	if( _oMenuNavigation.bUp )
	{
		--_nResourceMenuSelection;
		if( 0 > _nResourceMenuSelection )
		{
			_nResourceMenuSelection = 0;
		}
	}
	else if( _oMenuNavigation.bDown )
	{
		++_nResourceMenuSelection;
		if( (s32)_uFiltered <= _nResourceMenuSelection )
		{
			_nResourceMenuSelection = ( _uFiltered - 1 );
		}
	}
	else if( _oMenuNavigation.bLeft )
	{
		_nResourceMenuSelection -= _MENU_LINES_PER_COLS;
		if( 0 > _nResourceMenuSelection )
		{
			_nResourceMenuSelection += _MENU_LINES_PER_COLS;
		}
	}
	else if( _oMenuNavigation.bRight )
	{
		_nResourceMenuSelection += _MENU_LINES_PER_COLS;
		if( (s32)_uFiltered <= _nResourceMenuSelection )
		{
			if( ( _uFiltered / _MENU_LINES_PER_COLS ) == ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) )
			{
				_nResourceMenuSelection = ( (s32)_uFiltered - 1 );
			}
			else
			{
				_nResourceMenuSelection -= _MENU_LINES_PER_COLS;
			}
		}
	}

	strcpy( _pszCurrentSelection, _papszFiltered[ _nResourceMenuSelection ] );
	//
	////

	////
	//
	if( ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) < (s32)( _uFirstIndexOnCurrentPage / _MENU_LINES_PER_COLS ) )
	{
		do
		{
			_uFirstIndexOnCurrentPage -= _MENU_LINES_PER_COLS;

		} while( ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) < (s32)( _uFirstIndexOnCurrentPage / _MENU_LINES_PER_COLS ) );
	}
	else if( ( ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) - ( _uFirstIndexOnCurrentPage / _MENU_LINES_PER_COLS ) ) >= _MENU_COLS_PER_WINDOW )
	{
		do
		{
			_uFirstIndexOnCurrentPage += _MENU_LINES_PER_COLS;

		} while( ( ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) - ( _uFirstIndexOnCurrentPage / _MENU_LINES_PER_COLS ) ) >= _MENU_COLS_PER_WINDOW );
	}
	//
	////

	////
	//
	if( _oMenuNavigation.bSelect )
	{
		if( _MENU_SELECT_WORLD == _eCurrentMenu )
		{
			////
			//
			_SAFE_FDELETE( _poAnimCombiner );
			_SAFE_FDELETE( _poAnimInst );
			_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
			_ohResFrameSubject  = fres_GetFrame();
			_ohResFrameAnim     = NULL;
			_poCurrentAnimation = NULL;
			//
			////

			_pszCurrentSubjectName = _papszFiltered[ _nResourceMenuSelection ];

			////
			//
			fvid_Swap();
			fvid_Begin();
			fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
			ftext_DebugPrintf( 0.5f, 0.35f, "~w1~ac~c59000099Loading world, please wait." );
			fvid_End();
			//
			////

			FDS_StreamMgr.Uninit();
			FDS_StreamMgr.Init( 1024 );	//ARG
	
			_pCurrentSubject = fresload_Load( FWORLD_RESTYPE, _pszCurrentSubjectName );
			if( _pCurrentSubject )
			{
				_oSettings.oeResourceID = _RESOURCE_TYPE_WORLD;
				_bShowMenu              = FALSE;
				_bShowError             = FALSE;
				_eMovementType          = _MOVEMENT_TYPE_CAMERA_MOVE;

				strcpy( _oSettings.szResourceName, _pszCurrentSubjectName );

				fang_MemZero( _oSettings.afWSCameraXYZHP, sizeof( _oSettings.afWSCameraXYZHP ) );
				_fZSpeed = 80.75f;
				_oMeshXfm.Identity();

				_oMtx43Frame.Identity();
				_oVec3Frame.Zero();

				_CountStartPoint();
				_MoveCameraToNextStartPoint();
			}
			else
			{
				fclib_strcpy( _szErrorMessage, "Couldn't load world." );
				_bShowError         = TRUE;
				_bPreviousShowError = FALSE;
				_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
			}
		}
		else if( _MENU_SELECT_MESH == _eCurrentMenu )
		{
			////
			//
			_SAFE_FDELETE( _poAnimCombiner );
			_SAFE_FDELETE( _poAnimInst );
			_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
			_ohResFrameSubject  = fres_GetFrame();
			_ohResFrameAnim     = NULL;
			_poCurrentAnimation = NULL;
			//
			////

//ARG - >>>>>
#if FANG_PLATFORM_PS2
			fvid_Swap();
			fvid_Begin();
			fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
			fvid_End();
#endif
//ARG - <<<<<

			_pszCurrentSubjectName = _papszFiltered[ _nResourceMenuSelection ];
			_pCurrentSubject       = fresload_Load( FMESH_RESTYPE, _pszCurrentSubjectName );

			if( _pCurrentSubject )
			{
				_oSettings.oeResourceID = _RESOURCE_TYPE_MESH;
				_bShowMenu              = FALSE;
				_bShowError             = FALSE;
				_eMovementType          = _MOVEMENT_TYPE_MESH_MOVE;

				strcpy( _oSettings.szResourceName, _pszCurrentSubjectName );

				_oCameraXfm.Identity();
				_oMtx43Frame.Identity();
				_oVec3Frame.Zero();

				fang_MemZero( _oSettings.afWSCameraXYZHP, sizeof( _oSettings.afWSCameraXYZHP ) );

				_oMeshInst.Init( (FMesh_t *)_pCurrentSubject );
				_oMtx43Frame.Identity();
				_oVec3Frame.Zero();
				_oVec3Frame.y = - _oMeshInst.m_BoundSphere_MS.m_Pos.y;
				_oVec3Frame.z = _oMeshInst.m_BoundSphere_MS.m_fRadius * 2.0f;

#if FANG_PLATFORM_GC
	DEVPRINTF( "[ debug ] _oMeshInst.m_BoundSphere_MS.m_Pos.y: %f, _oMeshInst.m_BoundSphere_MS.m_fRadius: %f\n", _oMeshInst.m_BoundSphere_MS.m_Pos.y, _oMeshInst.m_BoundSphere_MS.m_fRadius );
#endif

				_fZSpeed = _oMeshInst.m_BoundSphere_MS.m_fRadius * 8.0f;
				_SetDefaultDirLight();
			}
			else
			{
				fclib_strcpy( _szErrorMessage, "Couldn't load mesh." );
				_bShowError         = TRUE;
				_bPreviousShowError = FALSE;
				_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
			}
		}
		else if( _MENU_SELECT_ANIMATION == _eCurrentMenu )
		{
			////
			//
			_SAFE_FDELETE( _poAnimCombiner );
			_SAFE_FDELETE( _poAnimInst );
			_SAFE_FRES_RELEASEFRAME( _ohResFrameAnim );
			_ohResFrameAnim = fres_GetFrame();
			//
			////

			_pszCurrentAnimationName = _papszFiltered[ _nResourceMenuSelection ];
			_poCurrentAnimation      = (FAnim_t *)fresload_Load( FANIM_RESNAME, _pszCurrentAnimationName );

			_poAnimInst     = fnew CFAnimInst;
			_poAnimCombiner = fnew CFAnimCombiner;

			if( _poCurrentAnimation && _poAnimInst && _poAnimCombiner )
			{
				_bShowMenu  = FALSE;
				_bShowError = FALSE;

				strcpy( _oSettings.szAnimationName, _pszCurrentAnimationName );

				_poAnimInst->Create( _poCurrentAnimation );
				_poAnimCombiner->CreateSimple( _poAnimInst, &_oMeshInst );

				_poAnimInst->UpdateUnitTime( 0.0f );
			}
			else
			{
				fclib_strcpy( _szErrorMessage, "Couldn't load animation." );
				_bShowError         = TRUE;
				_bPreviousShowError = FALSE;

				_SAFE_FDELETE( _poAnimCombiner );
				_SAFE_FDELETE( _poAnimInst );
				_SAFE_FRES_RELEASEFRAME( _ohResFrameAnim );
				_poCurrentAnimation = NULL;
			}
		}
		else if( _MENU_SELECT_TEXTURE == _eCurrentMenu )
		{
			////
			//
			_SAFE_FDELETE( _poAnimCombiner );
			_SAFE_FDELETE( _poAnimInst );
			_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
			_ohResFrameSubject  = fres_GetFrame();
			_ohResFrameAnim     = NULL;
			_poCurrentAnimation = NULL;
			//
			////

//ARG - >>>>>
#if FANG_PLATFORM_PS2
			EditMode = FALSE;	// NS Turn off global filter kludge before loading texture

			fvid_Swap();
			fvid_Begin();
			fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
			fvid_End();
#endif
//ARG - <<<<<

			_pszCurrentSubjectName = _papszFiltered[ _nResourceMenuSelection ];
			_pCurrentSubject       = fresload_Load( FTEX_RESNAME, _pszCurrentSubjectName );

			if( _pCurrentSubject )
			{
				_oTexInst.SetTexDef( ((FTexDef_t *)_pCurrentSubject) );

				_oSettings.oeResourceID = _RESOURCE_TYPE_TEXTURE;
				_bShowMenu              = FALSE;
				_bShowError             = FALSE;

				strcpy( _oSettings.szResourceName, _pszCurrentSubjectName );

				_eMovementType = _MOVEMENT_TYPE_MESH_MOVE;
				_oMtx43Frame.Identity();
				_oVec3Frame.Zero();
				_fZSpeed      = 2.0f * FMATH_MAX( ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross, ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown );
//ARG - >>>>>
#if FANG_PLATFORM_PS2
				_oVec3Frame.z = ( _poMainViewport->Res.x * 0.5f * _SCALE_TEXTURE );
				_aoTextureVerts[ 0 ].Pos_MS.Set( ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f * _SCALE_TEXTURE ), ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f * _SCALE_TEXTURE ), 0.0f );
				_aoTextureVerts[ 1 ].Pos_MS.Set( ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f * _SCALE_TEXTURE ), ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f * _SCALE_TEXTURE ), 0.0f );
				_aoTextureVerts[ 2 ].Pos_MS.Set( ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f * _SCALE_TEXTURE ), ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f * _SCALE_TEXTURE ), 0.0f );
				_aoTextureVerts[ 3 ].Pos_MS.Set( ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f * _SCALE_TEXTURE ), ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f * _SCALE_TEXTURE ), 0.0f );
#else
//ARG - <<<<<
				_oVec3Frame.z = ( _poMainViewport->Res.x * 0.5f );
				_aoTextureVerts[ 0 ].Pos_MS.Set( ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f ), ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f ), 0.0f );
				_aoTextureVerts[ 1 ].Pos_MS.Set( ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f ), ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f ), 0.0f );
				_aoTextureVerts[ 2 ].Pos_MS.Set( ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f ), ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f ), 0.0f );
				_aoTextureVerts[ 3 ].Pos_MS.Set( ( + ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsAcross * 0.5f ), ( - ((FTexDef_t *)_pCurrentSubject)->TexInfo.nTexelsDown * 0.5f ), 0.0f );
#endif	//ARG
				_oCameraXfm.Identity();
				_SetDefaultDirLight();
			}
			else
			{
				fclib_strcpy( _szErrorMessage, "Couldn't load texture." );
				_bShowError         = TRUE;
				_bPreviousShowError = FALSE;
				_SAFE_FRES_RELEASEFRAME( _ohResFrameSubject );
			}
		}
	}

} // _ManageResourceMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageColorMenu( void )
{
	if( _oMenuNavigation.bUp || _oMenuNavigation.bAlternateUp )
	{
		--_nColorMenuSelection;
		if( _nColorMenuSelection < 0 )
		{
			_nColorMenuSelection = _COLOR_MENU_NUM_ENTIRES - 1;
		}
	}
	else if( _oMenuNavigation.bDown || _oMenuNavigation.bAlternateDown )
	{
		++_nColorMenuSelection;
		if( _nColorMenuSelection >= _COLOR_MENU_NUM_ENTIRES )
		{
			_nColorMenuSelection = 0;
		}
	}

	if( _oMenuNavigation.bLeft || _oMenuNavigation.bAlternateLeft )
	{
		_fTemp = _pafCurrentColor[ _nColorMenuSelection ];
		_pafCurrentColor[ _nColorMenuSelection ] -= _MOVEMENT_SPEED_SLIDER;

		if( 0.0f > _pafCurrentColor[ _nColorMenuSelection ] )
		{
			_pafCurrentColor[ _nColorMenuSelection ] = 0.0f;
		}

		if( ( 3 == _nColorMenuSelection ) && ( 0.0f < _fTemp ) )
		{
			_pafCurrentColor[ 0 ] = ( _pafCurrentColor[ 0 ] * ( _pafCurrentColor[ 3 ] / _fTemp ) );
			_pafCurrentColor[ 1 ] = ( _pafCurrentColor[ 1 ] * ( _pafCurrentColor[ 3 ] / _fTemp ) );
			_pafCurrentColor[ 2 ] = ( _pafCurrentColor[ 2 ] * ( _pafCurrentColor[ 3 ] / _fTemp ) );

			if( 0.0f > _pafCurrentColor[ 0 ] )
			{
				_pafCurrentColor[ 0 ] = 0.0f;
			}
			if( 0.0f > _pafCurrentColor[ 1 ] )
			{
				_pafCurrentColor[ 1 ] = 0.0f;
			}
			if( 0.0f > _pafCurrentColor[ 2 ] )
			{
				_pafCurrentColor[ 2 ] = 0.0f;
			}
		}
		else
		{
			_pafCurrentColor[ 3 ] = ( ( _pafCurrentColor[ 0 ] + _pafCurrentColor[ 1 ] + _pafCurrentColor[ 2 ] ) / 3.0f );
		}
	}
	else if( _oMenuNavigation.bRight || _oMenuNavigation.bAlternateRight )
	{
		_fTemp = _pafCurrentColor[ _nColorMenuSelection ];
		_pafCurrentColor[ _nColorMenuSelection ] += _MOVEMENT_SPEED_SLIDER;

		if( 1.0f < _pafCurrentColor[ _nColorMenuSelection ] )
		{
			_pafCurrentColor[ _nColorMenuSelection ] = 1.0f;
		}

		if( 3 == _nColorMenuSelection )
		{
			if( 0.001f < _fTemp )
			{
				_pafCurrentColor[ 0 ] = ( _pafCurrentColor[ 0 ] * ( _pafCurrentColor[ 3 ] / _fTemp ) );
				_pafCurrentColor[ 1 ] = ( _pafCurrentColor[ 1 ] * ( _pafCurrentColor[ 3 ] / _fTemp ) );
				_pafCurrentColor[ 2 ] = ( _pafCurrentColor[ 2 ] * ( _pafCurrentColor[ 3 ] / _fTemp ) );

				if( 1.0f < _pafCurrentColor[ 0 ] )
				{
					_pafCurrentColor[ 0 ] = 1.0f;
				}
				if( 1.0f < _pafCurrentColor[ 1 ] )
				{
					_pafCurrentColor[ 1 ] = 1.0f;
				}
				if( 1.0f < _pafCurrentColor[ 2 ] )
				{
					_pafCurrentColor[ 2 ] = 1.0f;
				}
			}
			else
			{
				_pafCurrentColor[ 0 ] = _pafCurrentColor[ 1 ] = _pafCurrentColor[ 2 ] = _MOVEMENT_SPEED_SLIDER;
			}
		}
		else
		{
			_pafCurrentColor[ 3 ] = ( ( _pafCurrentColor[ 0 ] + _pafCurrentColor[ 1 ] + _pafCurrentColor[ 2 ] ) / 3.0f );
		}
	}

} // _ManageColorMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageWorldSubject( void )
{
	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_CYCLE_VIEW ]->uLatches )
	{
		_MoveCameraToNextStartPoint();
		return;
	}

	_oTempVec3MS.Set(	( FLoop_fPreviousLoopSecs * _fZSpeed * _oSubjectNavigation.fX ),
						( FLoop_fPreviousLoopSecs * _fZSpeed * _oSubjectNavigation.fY ),
						( FLoop_fPreviousLoopSecs * _fZSpeed * _oSubjectNavigation.fZ ) );

	_oCameraXfm.TransformPointR( _oTempVec3WS, _oTempVec3MS );

	_oSettings.afWSCameraXYZHP[ 0 ] = _oTempVec3WS.x;
	_oSettings.afWSCameraXYZHP[ 1 ] = _oTempVec3WS.y;
	_oSettings.afWSCameraXYZHP[ 2 ] = _oTempVec3WS.z;
	_oSettings.afWSCameraXYZHP[ 3 ] += ( FLoop_fPreviousLoopSecs * _MOVEMENT_SPEED_SUBJECT_ROTATION * _oSubjectNavigation.fHeading );
	_oSettings.afWSCameraXYZHP[ 4 ] += ( FLoop_fPreviousLoopSecs * _MOVEMENT_SPEED_SUBJECT_ROTATION * _oSubjectNavigation.fPitch );

	FMATH_CLAMP( _oSettings.afWSCameraXYZHP[ 4 ], _CAMERA_PITCH_MIN, _CAMERA_PITCH_MAX );

	_oCameraXfm.BuildInvRotYXZ_XlatFromPoint( _oSettings.afWSCameraXYZHP[ 3 ], _oSettings.afWSCameraXYZHP[ 4 ], 0.0f, _oSettings.afWSCameraXYZHP[ 0 ], _oSettings.afWSCameraXYZHP[ 1 ], _oSettings.afWSCameraXYZHP[ 2 ] );

} // _ManageWorldSubject

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageMeshSubject( void )
{
	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_CYCLE_VIEW ]->uLatches )
	{
		if( _MOVEMENT_TYPE_MESH_MOVE == _eMovementType )
		{
			_oMtx43Frame.Identity();
			_oVec3Frame.Zero();
			_oVec3Frame.y = - _oMeshInst.m_BoundSphere_MS.m_Pos.y;
			_oVec3Frame.z = _oMeshInst.m_BoundSphere_MS.m_fRadius * 2.0f;
		}
		else if( _MOVEMENT_TYPE_DIR_LIGHT_ORIENTATION == _eMovementType )
		{
			_SetDefaultDirLight();
		}
	}

	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_CYCLE_MOVEMENT ]->uLatches )
	{
		_eMovementType = (_MovementTypeID_e)( ( (u32)_eMovementType + 1 ) & 1 );
	}

	if( _MOVEMENT_TYPE_DIR_LIGHT_ORIENTATION == _eMovementType )
	{
		_fTemp = FLoop_fPreviousLoopSecs * _MOVEMENT_SPEED_SUBJECT_ROTATION;
		_oSettings.afWSDirectionalHP[ 0 ] += ( _fTemp * _oSubjectNavigation.fHeading );
		_oSettings.afWSDirectionalHP[ 1 ] += ( - ( _fTemp * _oSubjectNavigation.fPitch ) );

		_oTempVec3.x = _DEFAULT_DIRECTIONAL_LIGHT_X;
		_oTempVec3.y = _DEFAULT_DIRECTIONAL_LIGHT_Y;
		_oTempVec3.z = _DEFAULT_DIRECTIONAL_LIGHT_Z;

		_oTempVec3.RotateY( _oSettings.afWSDirectionalHP[ 0 ] );
		_oTempVec3.RotateX( _oSettings.afWSDirectionalHP[ 1 ] );

		_oSettings.afWSDirectionalXYZ[ 0 ] = _oTempVec3.x;
		_oSettings.afWSDirectionalXYZ[ 1 ] = _oTempVec3.y;
		_oSettings.afWSDirectionalXYZ[ 2 ] = _oTempVec3.z;
	}
	else if( _MOVEMENT_TYPE_MESH_MOVE == _eMovementType )
	{
		_fTemp = FLoop_fPreviousLoopSecs * _MOVEMENT_SPEED_SUBJECT_ROTATION;
		_oMtx43Frame.RotateX( - ( _fTemp * _oSubjectNavigation.fPitch ) );
		_oMtx43Frame.RotateY( - ( _fTemp * _oSubjectNavigation.fHeading ) );
		_oVec3Frame.z += ( - ( FLoop_fPreviousLoopSecs * _fZSpeed * _oSubjectNavigation.fZ ) );
		FMATH_CLAMP( _oVec3Frame.z, 1.0f, ( _FARPLANE - ( _FARPLANE * 0.01f ) ) );

		//// Build XFM from orientation and position.
		//
		_oTempMtx43.Identity();
		_oTempMtx43.m_vPos = _oVec3Frame;
		_oTempMtx43.m33 = _oMtx43Frame.m33;
		_oMeshXfm.BuildFromMtx( _oTempMtx43 );
		//
		////
	}

} // _ManageMeshSubject

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ManageTextureSubject( void )
{
	if( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT & _apoSamples[ _ACTION_CYCLE_VIEW ]->uLatches )
	{
		_oMtx43Frame.Identity();
		_oVec3Frame.Zero();
//ARG - >>>>>
#if FANG_PLATFORM_PS2
		_oVec3Frame.z = ( _poMainViewport->Res.x * 0.5f * _SCALE_TEXTURE );
#else
//ARG - <<<<<
		_oVec3Frame.z = ( _poMainViewport->Res.x * 0.5f );
#endif	//ARG
	}

	_oTempMtx43 = _oMtx43Frame;

	_fTemp = ( - ( FLoop_fPreviousLoopSecs * _MOVEMENT_SPEED_SUBJECT_ROTATION ) );
	_oMtx43Frame.RotateX( ( _fTemp * _oSubjectNavigation.fPitch ) );
	_oMtx43Frame.RotateY( ( _fTemp * _oSubjectNavigation.fHeading ) );

	if( _oMtx43Frame.m33.m_vZ.z < 0.0f)
	{
		_oMtx43Frame = _oTempMtx43;
	}

//ARG - >>>>>
#if FANG_PLATFORM_PS2
	_oVec3Frame.z += ( - ( FLoop_fPreviousLoopSecs * _fZSpeed * _oSubjectNavigation.fZ ) * _SCALE_TEXTURE );
#else
//ARG - <<<<<
	_oVec3Frame.z += ( - ( FLoop_fPreviousLoopSecs * _fZSpeed * _oSubjectNavigation.fZ ) );
#endif	//ARG
	FMATH_CLAMP( _oVec3Frame.z, 1.0f, ( _FARPLANE - ( _FARPLANE * 0.01f ) ) );

	//// Build XFM from orientation and position.
	//
	_oTempMtx43.Identity();
	_oTempMtx43.m_vPos = _oVec3Frame;
	_oTempMtx43.m33 = _oMtx43Frame.m33;
	_oMeshXfm.BuildFromMtx( _oTempMtx43 );
	//
	////

} // _ManageTextureSubject

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _BuildFilteredList( void )
{
	u32 uMatches, uIndex, uTemp;

	_uFiltered = 0;

	for( uIndex = 0; uIndex < _uCurrentResourceListLength; ++uIndex )
	{
		uMatches = 0;

		for( uTemp = 0; uTemp < _RESOURCE_NAME_LENGTH; ++uTemp )
		{
			if( ( _FILTER_ANY_CHAR == _pszCurrentFilter[ uTemp ] ) ||
				( _paszCurrentResourceList[ uIndex ][ uTemp ] == _pszCurrentFilter[ uTemp ] ) )
			{
				++uMatches;
			}
		}

		if( _RESOURCE_NAME_LENGTH == uMatches )
		{
			_papszFiltered[ _uFiltered++ ] = _paszCurrentResourceList[ uIndex ];
		}
	}

} // _BuildFilteredList

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _SetDefaultDirLight( void )
{
	_oSettings.afWSDirectionalHP[ 0 ]  = 0.0f;
	_oSettings.afWSDirectionalHP[ 1 ]  = 0.0f;

	_oSettings.afWSDirectionalXYZ[ 0 ] = _DEFAULT_DIRECTIONAL_LIGHT_X;
	_oSettings.afWSDirectionalXYZ[ 1 ] = _DEFAULT_DIRECTIONAL_LIGHT_Y;
	_oSettings.afWSDirectionalXYZ[ 2 ] = _DEFAULT_DIRECTIONAL_LIGHT_Z;
/*
	_oSettings.afWSDirectionalHP[ 0 ]  = ( - ( FMATH_PI - FMATH_QUARTER_PI ) );
	_oSettings.afWSDirectionalHP[ 1 ]  = ( - ( FMATH_PI - FMATH_QUARTER_PI ) );

	_oTempVec3.x = _DEFAULT_DIRECTIONAL_LIGHT_X;
	_oTempVec3.y = _DEFAULT_DIRECTIONAL_LIGHT_Y;
	_oTempVec3.z = _DEFAULT_DIRECTIONAL_LIGHT_Z;

	_oTempVec3.RotateY( FMATH_PI );
	_oTempVec3.RotateX( FMATH_PI );

	_oSettings.afWSDirectionalXYZ[ 0 ] = _oTempVec3.x;
	_oSettings.afWSDirectionalXYZ[ 1 ] = _oTempVec3.y;
	_oSettings.afWSDirectionalXYZ[ 2 ] = _oTempVec3.z;
*/
} // _SetDefaultDirLight

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _FindCurrentSelectionIndex( void )
{
	for( u32 uIndex = 0; uIndex < _uFiltered; ++uIndex )
	{
		if( ! strcmp( _pszCurrentSelection, _papszFiltered[ uIndex ] ) )
		{
			_nResourceMenuSelection = uIndex;
			break;
		}
	}

	FMATH_CLAMP( _nResourceMenuSelection, 0, ( (s32)_uFiltered - 1 ) );

} // _FindCurrentSelectionIndex

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _CountStartPoint( void )
{
	////
	//
	_uStartpoints = 0;
/*
	for( _uIndex = 0; _uIndex < FWorld_pWorld->nShapeCount; ++_uIndex )
	{
		if( FWORLD_SHAPETYPE_START_POINT == ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ _uIndex ])->nType )
		{
			++_uStartpoints;
		}
	}
*/
	//
	////

} // _CountStartPoint

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _MoveCameraToNextStartPoint( void )
{
	if( _uStartpoints )
	{
		++_uCurrentStartpoint %= _uStartpoints;

		u32 uTemp = 0;
/*
		for( u32 uIndex = 0; uIndex < FWorld_pWorld->nShapeCount; ++uIndex )
		{
			if( FWORLD_SHAPETYPE_START_POINT != ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ uIndex ])->nType )
			{
				continue;
			}

			if( uTemp != _uCurrentStartpoint )
			{
				++uTemp;
				continue;
			}

			_oMtx43Frame.Identity();
			_oMtx43Frame.SetRotationY( ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ uIndex ])->pSpawnPoint->m_fHeading );
			_oVec3Frame = ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ uIndex ])->pSpawnPoint->m_Pos;

			_oSettings.afWSCameraXYZHP[ 0 ] = ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ uIndex ])->pSpawnPoint->m_Pos.x;
			_oSettings.afWSCameraXYZHP[ 1 ] = ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ uIndex ])->pSpawnPoint->m_Pos.y + 5.0f;
			_oSettings.afWSCameraXYZHP[ 2 ] = ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ uIndex ])->pSpawnPoint->m_Pos.z;
			_oSettings.afWSCameraXYZHP[ 3 ] = ((FWorldShapeDef_t *)&FWorld_pWorld->pShapeArray[ uIndex ])->pSpawnPoint->m_fHeading;
			_oSettings.afWSCameraXYZHP[ 4 ] = 0.0f;

//			_oCameraXfm.BuildInvRotYXZ_XlatFromPoint( _oSettings.afWSCameraXYZHP[ 3 ], _oSettings.afWSCameraXYZHP[ 4 ], 0.0f, _oSettings.afWSCameraXYZHP[ 0 ], _oSettings.afWSCameraXYZHP[ 1 ], _oSettings.afWSCameraXYZHP[ 3 ] );
			_oCameraXfm.BuildRotYXZ_XlatFromPoint( _oSettings.afWSCameraXYZHP[ 3 ], _oSettings.afWSCameraXYZHP[ 4 ], 0.0f, _oSettings.afWSCameraXYZHP[ 0 ], _oSettings.afWSCameraXYZHP[ 1 ], _oSettings.afWSCameraXYZHP[ 3 ] );
#if 0
_oSettings.afWSCameraXYZHP[ 0 ] = _oCameraXfm.m_MtxF.m_vPos.x;
_oSettings.afWSCameraXYZHP[ 1 ] = _oCameraXfm.m_MtxF.m_vPos.y;
_oSettings.afWSCameraXYZHP[ 2 ] = _oCameraXfm.m_MtxF.m_vPos.z;
_oSettings.afWSCameraXYZHP[ 3 ] = _oCameraXfm.m_MtxF.heading??;
_oSettings.afWSCameraXYZHP[ 4 ] = 0.0f;
#endif
			break;
		}
*/
	}
	else
	{
		_oSettings.afWSCameraXYZHP[ 0 ] = 0.0f;
		_oSettings.afWSCameraXYZHP[ 1 ] = 5.0f;
		_oSettings.afWSCameraXYZHP[ 2 ] = 0.0f;
		_oSettings.afWSCameraXYZHP[ 3 ] = 0.0f;
		_oSettings.afWSCameraXYZHP[ 4 ] = 0.0f;

		_oCameraXfm.BuildInvRotYXZ_XlatFromPoint( 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f );
	}

} // _MoveCameraToNextStartPoint

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintHelpMainMenu( void )
{
#if FANG_PLATFORM_GC
	sprintf( _szTempOutput,	"~w1Main Menu Help\n"							\
							"\n"											\
							"\tToggle Main Menu.\n"							\
							"\t\t~c39395999[Start]~c59590099\n"				\
							"\n"											\
							"\tToggle Help\n"								\
							"\t\t~c39395999[Y]~c59590099\n"					\
							"\n"											\
							"\tBack.\n"										\
							"\t\t~c39395999[X]~c59590099\n"					\
							"\n"											\
							"\tSelect\n"									\
							"\t\t~c39395999[A]~c59590099\n"					\
							"\n"											\
							"\tMove selection.\n"							\
							"\t\t~c39395999[Dpad]~c59590099 or\n"			\
							"\t\t~c39395999[Left Stick]~c59590099 or\n"		\
							"\t\t~c39395999[Right Stick]"
							);
#else
	sprintf( _szTempOutput,	"~w1Main Menu Help\n"							\
							"\n"											\
							"\tToggle Main Menu.\n"							\
							"\t\t~c39395999[Start]~c59590099\n"				\
							"\n"											\
							"\tToggle Help\n"								\
							"\t\t~c39395999[Y]~c59590099\n"					\
							"\n"											\
							"\tBack.\n"										\
							"\t\t~c39395999[B]~c59590099\n"					\
							"\n"											\
							"\tSelect\n"									\
							"\t\t~c39395999[A]~c59590099\n"					\
							"\n"											\
							"\tMove selection.\n"							\
							"\t\t~c39395999[Dpad]~c59590099 or\n"			\
							"\t\t~c39395999[Left Stick]~c59590099 or\n"		\
							"\t\t~c39395999[Right Stick]"
							);
#endif

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

} // _PrintHelpMainMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintHelpResourceMenu( void )
{
#if FANG_PLATFORM_GC
	sprintf( _szTempOutput,	"~w1Resource Selection Menu Help\n"			\
							"\n"										\
							"\tToggle Main Menu.\n"						\
							"\t\t~c39395999[Start]~c59590099\n"			\
							"\n"										\
							"\tToggle Help\n"							\
							"\t\t~c39395999[Y]~c59590099\n"				\
							"\n"										\
							"\tBack.\n"									\
							"\t\t~c39395999[X]~c59590099\n"				\
							"\n"										\
							"\tSelect\n\t\t~c39395999[A]~c59590099\n"	\
							"\n"										\
							"\tMove selection.\n"						\
							"\t\t~c39395999[Dpad]~c59590099 or\n"		\
							"\t\t~c39395999[Left Stick]~c59590099\n"	\
							"\n"										\
							"\tModify filter.\n"						\
							"\t\t~c39395999[Right Stick]~c59590099\n"	\
							"\n"										\
							"\tReset filter.\n"							\
							"\t\t~c39395999[B]~c59590099\n"				\
							"\n"										\
							"\tReset current letter of filter.\n"		\
							"\t\t~c39395999[Left Trigger]"
							);
#else
	sprintf( _szTempOutput,	"~w1Resource Selection Menu Help\n"			\
							"\n"										\
							"\tToggle Main Menu.\n"						\
							"\t\t~c39395999[Start]~c59590099\n"			\
							"\n"										\
							"\tToggle Help\n"							\
							"\t\t~c39395999[Y]~c59590099\n"				\
							"\n"										\
							"\tBack.\n"									\
							"\t\t~c39395999[B]~c59590099\n"				\
							"\n"										\
							"\tSelect\n\t\t~c39395999[A]~c59590099\n"	\
							"\n"										\
							"\tMove selection.\n"						\
							"\t\t~c39395999[Dpad]~c59590099 or\n"		\
							"\t\t~c39395999[Left Stick]~c59590099\n"	\
							"\n"										\
							"\tModify filter.\n"						\
							"\t\t~c39395999[Right Stick]~c59590099\n"	\
							"\n"										\
							"\tReset filter.\n"							\
							"\t\t~c39395999[X]~c59590099\n"				\
							"\n"										\
							"\tReset current letter of filter.\n"		\
							"\t\t~c39395999[Left Trigger]"
							);
#endif

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

} // _PrintHelpResourceMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintHelpColorMenu( void )
{
#if FANG_PLATFORM_GC
	sprintf( _szTempOutput,	"~w1Color Selection Menu Help\n"						\
							"\n"													\
							"\tToggle Main Menu.\n"									\
							"\t\t~c39395999[Start]~c59590099\n"						\
							"\n"													\
							"\tToggle Help\n"										\
							"\t\t~c39395999[Y]~c59590099\n"							\
							"\n"													\
							"\tBack.\n"												\
							"\t\t~c39395999[X]~c59590099\n"							\
							"\n"													\
							"\tSelect\n"											\
							"\t\t~c39395999[A]~c59590099\n"							\
							"\n"													\
							"\tMove selection.\n"									\
							"\t\t~c39395999[Dpad Up/Down]~c59590099 or\n"			\
							"\t\t~c39395999[Left Stick Up/Down]~c59590099 or\n"		\
							"\t\t~c39395999[Right Stick Up/Down]~c59590099\n\n"		\
							"\tModify color.\n"										\
							"\t\t~c39395999[Dpad Left/Right]~c59590099 or\n"		\
							"\t\t~c39395999[Left Stick Left/Right]~c59590099 or\n"	\
							"\t\t~c39395999[Right Stick Left/Right]"
							);
#else
	sprintf( _szTempOutput,	"~w1Color Selection Menu Help\n"						\
							"\n"													\
							"\tToggle Main Menu.\n"									\
							"\t\t~c39395999[Start]~c59590099\n"						\
							"\n"													\
							"\tToggle Help\n"										\
							"\t\t~c39395999[Y]~c59590099\n"							\
							"\n"													\
							"\tBack.\n"												\
							"\t\t~c39395999[B]~c59590099\n"							\
							"\n"													\
							"\tSelect\n"											\
							"\t\t~c39395999[A]~c59590099\n"							\
							"\n"													\
							"\tMove selection.\n"									\
							"\t\t~c39395999[Dpad Up/Down]~c59590099 or\n"			\
							"\t\t~c39395999[Left Stick Up/Down]~c59590099 or\n"		\
							"\t\t~c39395999[Right Stick Up/Down]~c59590099\n\n"		\
							"\tModify color.\n"										\
							"\t\t~c39395999[Dpad Left/Right]~c59590099 or\n"		\
							"\t\t~c39395999[Left Stick Left/Right]~c59590099 or\n"	\
							"\t\t~c39395999[Right Stick Left/Right]"
							);
#endif

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

} // _PrintHelpColorMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintHelpWorldDisplay( void )
{
#if FANG_PLATFORM_GC
	sprintf( _szTempOutput,	"~w1World Viewer Help\n"					\
							"\n"										\
							"\tToggle Main Menu.\n"						\
							"\t\t~c39395999[Start]~c59590099\n"			\
							"\n"										\
							"\tToggle Help\n"							\
							"\t\t~c39395999[Y]~c59590099\n"				\
							"\n"										\
							"\tBack.\n"									\
							"\t\t~c39395999[X]~c59590099\n"				\
							"\n"										\
							"\tCycle start points.\n"					\
							"\t\t~c39395999[A]~c59590099\n"				\
							"\n"										\
							"\tMove camera.\n"							\
							"\t\t~c39395999[Dpad]~c59590099 or\n"		\
							"\t\t~c39395999[Sticks]~c59590099 or\n"		\
							"\t\t~c39395999[Triggers]"
							);
#else
	sprintf( _szTempOutput,	"~w1World Viewer Help\n"					\
							"\n"										\
							"\tToggle Main Menu.\n"						\
							"\t\t~c39395999[Start]~c59590099\n"			\
							"\n"										\
							"\tToggle Help\n"							\
							"\t\t~c39395999[Y]~c59590099\n"				\
							"\n"										\
							"\tBack.\n"									\
							"\t\t~c39395999[B]~c59590099\n"				\
							"\n"										\
							"\tCycle start points.\n"					\
							"\t\t~c39395999[A]~c59590099\n"				\
							"\n"										\
							"\tMove camera.\n"							\
							"\t\t~c39395999[Dpad]~c59590099 or\n"		\
							"\t\t~c39395999[Sticks]~c59590099 or\n"		\
							"\t\t~c39395999[Triggers]"
							);
#endif

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

} // _PrintHelpWorldDisplay

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintHelpMeshDisplay( void )
{
#if FANG_PLATFORM_GC
	sprintf( _szTempOutput,	"~w1Mesh Viewer Help\n"						\
							"\n"										\
							"\tToggle Main Menu.\n"						\
							"\t\t~c39395999[Start]~c59590099\n"			\
							"\n"										\
							"\tToggle Help\n"							\
							"\t\t~c39395999[Y]~c59590099\n"				\
							"\n"										\
							"\tBack.\n"									\
							"\t\t~c39395999[X]~c59590099\n"				\
							"\n"										\
							"\tMove object.\n"							\
							"\t\t~c39395999[Dpad]~c59590099\n"			\
							"\t\t~c39395999[Sticks]~c59590099\n"		\
							"\n"										\
							"\tReset view.\n"							\
							"\t\t~c39395999[A]~c59590099\n"				\
							"\n"										\
							"\tToggle camera / directional light.\n"	\
							"\t\t~c39395999[B]"							\
							);
#else
	sprintf( _szTempOutput,	"~w1Mesh Viewer Help\n"						\
							"\n"										\
							"\tToggle Main Menu.\n"						\
							"\t\t~c39395999[Start]~c59590099\n"			\
							"\n"										\
							"\tToggle Help\n"							\
							"\t\t~c39395999[Y]~c59590099\n"				\
							"\n"										\
							"\tBack.\n"									\
							"\t\t~c39395999[B]~c59590099\n"				\
							"\n"										\
							"\tMove object.\n"							\
							"\t\t~c39395999[Dpad]~c59590099\n"			\
							"\t\t~c39395999[Sticks]~c59590099\n"		\
							"\n"										\
							"\tReset view.\n"							\
							"\t\t~c39395999[A]~c59590099\n"				\
							"\n"										\
							"\tToggle camera / directional light.\n"	\
							"\t\t~c39395999[X]"							\
							);
#endif

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

} // _PrintHelpMeshDisplay

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _PrintHelpTextureDisplay( void )
{
#if FANG_PLATFORM_GC
	sprintf( _szTempOutput,	"~w1Texture Viewer Help\n"				\
							"\n"									\
							"\tToggle Main Menu.\n"					\
							"\t\t~c39395999[Start]~c59590099\n"		\
							"\n"									\
							"\tToggle Help\n"						\
							"\t\t~c39395999[Y]~c59590099\n"			\
							"\n"									\
							"\tBack.\n"								\
							"\t\t~c39395999[X]~c59590099\n"			\
							"\n"									\
							"\tMove object.\n"						\
							"\t\t~c39395999[Dpad]~c59590099\n"		\
							"\t\t~c39395999[Sticks]~c59590099\n"	\
							"\n"									\
							"\tReset view.\n"						\
							"\t\t~c39395999[A]"						\
							);
#else
	sprintf( _szTempOutput,	"~w1Texture Viewer Help\n"				\
							"\n"									\
							"\tToggle Main Menu.\n"					\
							"\t\t~c39395999[Start]~c59590099\n"		\
							"\n"									\
							"\tToggle Help\n"						\
							"\t\t~c39395999[Y]~c59590099\n"			\
							"\n"									\
							"\tBack.\n"								\
							"\t\t~c39395999[B]~c59590099\n"			\
							"\n"									\
							"\tMove object.\n"						\
							"\t\t~c39395999[Dpad]~c59590099\n"		\
							"\t\t~c39395999[Sticks]~c59590099\n"	\
							"\n"									\
							"\tReset view.\n"						\
							"\t\t~c39395999[A]"						\
							);
#endif

	ftext_DebugPrintString( 0.075f, 0.075f, _szTempOutput );

} // _PrintHelpTextureDisplay

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//			_oCameraXfm.Identity();

/*
static CFXfm x, y, z, a;
static CFMtx43 _r1, _r2, oTemp;
static CFVec3 _t1;

x.BuildRotationX( _oSettings.fWSCameraX );
z.BuildTranslation( 0.0f, 0.0f, _oSettings.fWSCameraZ );

_prev.m_MtxF.m_vPos.z += _oSubjectNavigation.fZ;

/*
//_oVec3Frame.z += _oSubjectNavigation.fZ;

//_oTempMtx43.m_vPos = _oVec3Frame;
_oTempMtx43.m33 = _oMtx43Frame.m33;
_oCameraXfm.BuildFromMtx( _oTempMtx43 );
_oCameraXfm.m_MtxF.m_vPos.z = _oSettings.fWSCameraZ;
*/

//_oCameraXfm.m_MtxF.RotateX( _oSubjectNavigation.fPitch );
//_oCameraXfm.ReceiveProductOf( x, y );
/*
_r1.Identity();
_r1.SetRotationX( - _oSettings.fWSCameraX );
_r2.Identity();
_r2.SetRotationY( _oSettings.fWSCameraY );

oTemp = _r2 * _r1;

oTemp.m_vPos = _oCameraXfm.m_MtxF.m_vPos.Add( ( _oCameraXfm.m_MtxR.m_vZ.Mul( _oSubjectNavigation.fZ ) ) ).v3;

_oCameraXfm.BuildFromMtx( oTemp );
//_oCameraXfm.Invert();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static CFAnimCombinerConfig *_poAnimCombinerConfig;
static s32 _nAnimControlID;
static s32 _nAnimTapID;

_poAnimCombinerConfig = fnew CFAnimCombinerConfig;
if( ! _poAnimCombinerConfig )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

if( ! _poAnimCombinerConfig->BeginCreation( "Mixer0", CFAnimMixer::TYPE_BLENDER ) )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

if( ! _poAnimCombinerConfig->AddTap( "Tap0", "Mixer0", 0 ) )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

if( ! _poAnimCombinerConfig->EndCreation() )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

_poAnimCombiner = fnew CFAnimCombiner;
if( ! _poAnimCombiner )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

if( ! _poAnimCombiner->Create( _poAnimCombinerConfig, &_oMeshInst ) )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

_poAnimInst = fnew CFAnimInst;
if( ! _poAnimInst )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

if( ! _poAnimInst->Create( _poCurrentAnimation ) )
{
	sprintf( _szErrorMessage, "Couldn't load animation. (%u)", __LINE__ );
	_bShowError = TRUE;
	_bPreviousShowError = FALSE;
}

_nAnimControlID = _poAnimCombiner->GetControlID( "Mixer0" );
_nAnimTapID = _poAnimCombiner->GetTapID( "Tap0" );
FASSERT( _nAnimControlID != -1 );
FASSERT( _nAnimTapID != -1 );

_poAnimCombiner->AttachToTap( _nAnimTapID, _poAnimInst );
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////
//
static BOOL _bAmbientLightWorld;
static f32 _afAmbientLightWorldColor[ 3 ];
static u32 _uDirLightWorld;
static FLight_t _aoDirLightWorld[ 3 ];

static FWorldNode_t *_poWorldNode;
//
////

				////
				//
				_bAmbientLightWorld = FALSE;
				_uDirLightWorld = 0;

				for( _uIndex = 0; _uIndex < FWorld_MeshInst.m_pMesh->nLightCount; ++_uIndex )
				{
					if( ( FLIGHT_TYPE_DIR == FWorld_MeshInst.m_pMesh->pLightArray[ _uIndex ].LightInit.nType ) &&
						( 3 > _uDirLightWorld ) )
					{
						flight_Init( &_aoDirLightWorld[ _uDirLightWorld ], &FWorld_MeshInst.m_pMesh->pLightArray[ _uIndex ].LightInit );
						++_uDirLightWorld;
					}
					else if( ( FLIGHT_TYPE_AMBIENT == FWorld_MeshInst.m_pMesh->pLightArray[ _uIndex ].LightInit.nType ) &&
							 ( ! _bAmbientLightWorld ) )
					{
						_afAmbientLightWorldColor[ 0 ] = FWorld_MeshInst.m_pMesh->pLightArray[ _uIndex ].LightInit.Motif.ColorRGB.fRed;
						_afAmbientLightWorldColor[ 1 ] = FWorld_MeshInst.m_pMesh->pLightArray[ _uIndex ].LightInit.Motif.ColorRGB.fGreen;
						_afAmbientLightWorldColor[ 2 ] = FWorld_MeshInst.m_pMesh->pLightArray[ _uIndex ].LightInit.Motif.ColorRGB.fBlue;
						_bAmbientLightWorld = TRUE;
					}
				}
				//
				////


	if( _RESOURCE_TYPE_WORLD == _oSettings.oeResourceID )
	{
		if( _bAmbientLightWorld )
		{
			fworld_Ambient_Set( _afAmbientLightWorldColor[ 0 ], _afAmbientLightWorldColor[ 1 ], _afAmbientLightWorldColor[ 2 ] );
		}
		else
		{
			if( _oSettings.bAmbient )
			{
				fworld_Ambient_Set( _oSettings.afColorAmbientRGBI[ 0 ], _oSettings.afColorAmbientRGBI[ 1 ], _oSettings.afColorAmbientRGBI[ 2 ], _oSettings.afColorAmbientRGBI[ 3 ] );
			}
		}

		if( _uDirLightWorld )
		{
			for( _uIndex = 0; _uIndex < _uDirLightWorld; ++_uIndex )
			{
				fmesh_AddLight( &( _aoDirLightWorld[ _uIndex ] ) );
			}
		}
		else
		{
			if( _oSettings.bDirectional )
			{
				flight_InitDirLight( &_oDirectionalLight, &_oDirLight );
				flight_SetColor( &_oDirectionalLight, _oSettings.afColorDirectionalRGBI[ 0 ], _oSettings.afColorDirectionalRGBI[ 1 ], _oSettings.afColorDirectionalRGBI[ 2 ], _oSettings.afColorDirectionalRGBI[ 3 ] );
				fmesh_AddLight( &_oDirectionalLight );
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if( 0 < _uFiltered )
		{
			////
			//
			if( _oMenuNavigation.bUp )
			{
				_uIndex = _nResourceMenuSelection--;
				if( ( (u32)_nResourceMenuSelection / _MENU_LINES_PER_COLS ) != ( _uIndex / _MENU_LINES_PER_COLS ) )
				{
					_nResourceMenuSelection = ( _uIndex - ( ( _MENU_LINES_PER_COLS * ( _MENU_COLS_PER_WINDOW - 1 ) ) + 1 ) );
					if( 0 > _nResourceMenuSelection )
					{
						_nResourceMenuSelection = ( _uFiltered - 1 );
						while( ( ( _uIndex / _MENU_LINES_PER_COLS ) % _MENU_COLS_PER_WINDOW ) < (u32)( ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) % _MENU_COLS_PER_WINDOW ) ) --_nResourceMenuSelection;
					}
				}
			}
			else if( _oMenuNavigation.bDown )
			{
				_uIndex = _nResourceMenuSelection++;
				if( ( _uFiltered - 1 ) < (u32)_nResourceMenuSelection )
				{
					_nResourceMenuSelection = ( ( ( _uIndex / _MENU_LINES_PER_COLS ) % _MENU_COLS_PER_WINDOW ) * _MENU_LINES_PER_COLS );
				}
				else if( ( _nResourceMenuSelection / _MENU_LINES_PER_COLS ) != ( _uIndex / _MENU_LINES_PER_COLS ) )
				{
					_nResourceMenuSelection = ( _uIndex + ( ( _MENU_LINES_PER_COLS * ( _MENU_COLS_PER_WINDOW - 1 ) ) + 1 ) );
					if( ( _uFiltered - 1 ) < (u32)_nResourceMenuSelection )
					{
						if( ( _uIndex / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) == ( ( _uFiltered - 1 ) / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) )
						{
							_nResourceMenuSelection = ( ( ( _uIndex / _MENU_LINES_PER_COLS ) % _MENU_COLS_PER_WINDOW ) * _MENU_LINES_PER_COLS );
						}
						else
						{
							_nResourceMenuSelection = ( _uFiltered - 1 );
							while( _nResourceMenuSelection % _MENU_LINES_PER_COLS ) --_nResourceMenuSelection;
						}
					}
				}
			}
			//
			////

			////
			//
			if( _oMenuNavigation.bLeft )
			{
				_uIndex = _nResourceMenuSelection;
				_nResourceMenuSelection -= _MENU_LINES_PER_COLS;
				if( 0 > _nResourceMenuSelection )
				{
					_nResourceMenuSelection = ( _uIndex + ( _MENU_LINES_PER_COLS * ( _MENU_COLS_PER_WINDOW - 1 ) ) );
					while( ( _uFiltered - 1 ) < (u32)_nResourceMenuSelection ) _nResourceMenuSelection -= _MENU_LINES_PER_COLS;
				}
				else if( ( (u32)_nResourceMenuSelection / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) != ( _uIndex / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) )
				{
					_nResourceMenuSelection = ( _uIndex + ( _MENU_LINES_PER_COLS * ( _MENU_COLS_PER_WINDOW - 1 ) ) );
					while( ( _uFiltered - 1 ) < (u32)_nResourceMenuSelection ) _nResourceMenuSelection -= _MENU_LINES_PER_COLS;
				}
			}
			else if( _oMenuNavigation.bRight )
			{
				_uIndex = _nResourceMenuSelection;
				_nResourceMenuSelection += _MENU_LINES_PER_COLS;
				if( ( _uFiltered - 1 ) < (u32)_nResourceMenuSelection )
				{
					_nResourceMenuSelection = ( ( ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) * ( _uIndex / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) ) + ( _uIndex % _MENU_LINES_PER_COLS ) );
				}
				else if( ( _nResourceMenuSelection / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) != ( _uIndex / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) )
				{
					_nResourceMenuSelection = ( _uIndex - ( _MENU_LINES_PER_COLS * ( _MENU_COLS_PER_WINDOW - 1 ) ) );
				}
			}
			//
			////

			////
			//
			if( _oMenuNavigation.bPageUp )
			{
				_uIndex = _nResourceMenuSelection;
				_nResourceMenuSelection -= ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW );
				if( 0 > _nResourceMenuSelection )
				{
					_nResourceMenuSelection = ( _uIndex + ( ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) * ( ( _uFiltered - 1 ) / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) ) );
					if( (u32)_nResourceMenuSelection > ( _uFiltered - 1 ) )
					{
						_nResourceMenuSelection = ( _uFiltered - 1 );
					}
				}
			}
			else if( _oMenuNavigation.bPageDown )
			{
				_uIndex = _nResourceMenuSelection;
				_nResourceMenuSelection += ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW );
				if( _nResourceMenuSelection > ( (s32)_uFiltered - 1 ) )
				{
					if( ( _uIndex / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) == ( ( _uFiltered - 1 ) / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) )
					{
						_nResourceMenuSelection = ( ( _uIndex % _MENU_LINES_PER_COLS ) + ( ( ( _uIndex / _MENU_LINES_PER_COLS ) % _MENU_COLS_PER_WINDOW ) * _MENU_LINES_PER_COLS ) );
					}
					else
					{
						_nResourceMenuSelection = ( _uFiltered - 1 );
					}
				}
			}
			//
			////
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	_uFirstIndexOnCurrentPage = ( ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) * ( _nResourceMenuSelection / ( _MENU_LINES_PER_COLS * _MENU_COLS_PER_WINDOW ) ) );

FMESH_RESTYPE
FANIM_RESNAME
FWORLD_RESTYPE
FTEX_RESNAME

_oMeshInst.m_Xfm.BuildRotYXZ_XlatFromPoint( _oSubjectNavigation.fHeading, _oSubjectNavigation.fPitch, 0.0f, 0.0f, 0.0f, _oSubjectNavigation.fZ );
_oMeshInst.m_Xfm *= _oMeshMovement;

_oMeshCumulativeDelta.Identity();

_oMeshDelta.BuildTranslation( 0.0f, 0.0f, _oSubjectNavigation.fZ );
_oMeshCumulativeDelta.ReceiveProductOf( _oMeshCumulativeDelta, _oMeshDelta );

_oMeshDelta.BuildRotationX( _oSubjectNavigation.fPitch );
_oMeshCumulativeDelta.ReceiveProductOf( _oMeshCumulativeDelta, _oMeshDelta );

_oMeshDelta.BuildRotationY( _oSubjectNavigation.fHeading );
_oMeshCumulativeDelta.ReceiveProductOf( _oMeshCumulativeDelta, _oMeshDelta );

_oMeshMovement.m_Xfm.BuildRotationY( _oSubjectNavigation.fHeading );
_oMeshMovement.BuildTranslation( 0.0f, 0.0f, _oSubjectNavigation.fZ );
_oMeshInst.m_Xfm.ReceiveProductOf( _oMeshMovement, _oMeshInst.m_Xfm );

//_oMeshInst.m_Xfm.BuildRotYXZ_XlatFromPoint( _oSubjectNavigation.fHeading, _oSubjectNavigation.fPitch, 0.0f, 0.0f, - _oMeshInst.m_BoundSphere_MS.m_Pos.y, _oMeshInst.m_BoundSphere_MS.m_fRadius * 2.0f );
//_oMeshInst.m_Xfm.BuildRotYXZ_XlatFromPoint( _oSubjectNavigation.fHeading, _oSubjectNavigation.fPitch, 0.0f, 0.0f, - _oMeshInst.m_BoundSphere_MS.m_fRadius, _oMeshInst.m_BoundSphere_MS.m_fRadius * 2.0f );

else
{
	FASSERT_NOW;
}
	//
	////

*/

//ARG - >>>>>
#if FANG_PLATFORM_PS2
static void PS2EditFilter(void)
{

				U8 l; S16 k;
				U8 mipfilter;

				mipfilter = ((FTexDef_t *)_pCurrentSubject)->pTexData->pPS2TexObj->MipFilter;
				((FTexDef_t *)_pCurrentSubject)->pTexData->pPS2TexObj->GetLK(&l,&k);

                if(gController[0].ButtonP & MMICONTROLLER_L2)
                        EditMode ^=1;

                if(!EditMode) return;

				if(gController[0].Button & MMICONTROLLER_R2)
				{
					if(gController[0].Button & MMICONTROLLER_R)
					{
						k++;
					}
					if(gController[0].Button & MMICONTROLLER_L)
					{
						k--;
					}
				}

				if(gController[0].Button & MMICONTROLLER_R1)
				{
					if(gController[0].ButtonP & MMICONTROLLER_R)
					{
						l++;
						l %= 3;
					}
					if(gController[0].ButtonP & MMICONTROLLER_L)
					{
						if(l)
							l--;
						else
							l = 2;
					}
				}


				if(gController[0].Button & MMICONTROLLER_L1)
				{
					if(gController[0].ButtonP & MMICONTROLLER_R)
					{
						mipfilter++;
						mipfilter %=6;
					}
					if(gController[0].ButtonP & MMICONTROLLER_L)
					{
						if(mipfilter)
							mipfilter--;
						else
							mipfilter = 5;
					}
				}


				((FTexDef_t *)_pCurrentSubject)->pTexData->pPS2TexObj->SetLK(l,k);
                ((FTexDef_t *)_pCurrentSubject)->pTexData->pPS2TexObj->MipFilter = (MMIMIPFILTER)mipfilter;

                EditFilter = mipfilter;
                EditL = l;
                EditK = k;

				ftext_DebugPrintf( 0.500f, 0.600f, "~w1~s2.00~acL=%u K=%d Mip=%u", l, k, mipfilter );
}
#endif
//ARG - <<<<<
