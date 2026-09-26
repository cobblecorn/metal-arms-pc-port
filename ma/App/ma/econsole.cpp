//////////////////////////////////////////////////////////////////////////////////////
// econsole.cpp - Remote control console entity object.
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
// 04/17/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "econsole.h"
#include "entity.h"
#include "fres.h"
#include "fresload.h"
#include "frenderer.h"
#include "fdraw.h"
#include "floop.h"
#include "fclib.h"
#include "fscriptsystem.h"
#include "gstring.h"
#include "bot.h"
#include "iteminst.h"
#include "player.h"
#include "botfx.h"
#include "GameCam.h"
#include "PauseScreen.h"
#include "botGlitch.h"
#include "BotDispenser.h"
#include "AlarmSys.h"
#include "eshield.h"
#include "botglitch.h"
#include "game.h"
#include "Ai/AIBrainMan.h"

#define _GIMME_CHIPS 0

#define _CONSOLE_SOUND_BANK				"p_console"
#define _BOTINFO_FILENAME				"e_console"
#define _ERROR_MESH						"gpdmwpnunkn"

#define _CHIP_MESH						"GR_1chips01"
#define _DEFAULT_STICK_MESH				"GOM_joyst01"
#define _DEFAULT_CONSOLE_MESH			"GOM_posses2"
#define _MILLOGO_TEXTURE				"MilLogo128x"
#define _DEFAULT_SOCKET_BONE_NAME		"chip"

#define _BOX_BONE_NAME					"triggerbox"
#define _JOYSTICK_BONE_NAME				"stick"

#define _DEFAULT_OPERATOR_BOX_MIN_X		-2.5f
#define _DEFAULT_OPERATOR_BOX_MAX_X		 2.5f
#define _DEFAULT_OPERATOR_BOX_MIN_Y		-0.5f
#define _DEFAULT_OPERATOR_BOX_MAX_Y		 1.0f
#define _DEFAULT_OPERATOR_BOX_MIN_Z		-2.5f
#define _DEFAULT_OPERATOR_BOX_MAX_Z		 2.5f
#define _DEFAULT_OPERATOR_BOX_DIM_X		(_DEFAULT_OPERATOR_BOX_MAX_X - _DEFAULT_OPERATOR_BOX_MIN_X)
#define _DEFAULT_OPERATOR_BOX_DIM_Y		(_DEFAULT_OPERATOR_BOX_MAX_Y - _DEFAULT_OPERATOR_BOX_MIN_Y)
#define _DEFAULT_OPERATOR_BOX_DIM_Z		(_DEFAULT_OPERATOR_BOX_MAX_Z - _DEFAULT_OPERATOR_BOX_MIN_Z)

//#if 1
//#define _STICK_X_PS2					-1.303f
//#define _STICK_Y_PS						3.068f
//#define _STICK_Z_PS2					1.308f
//#else
//#define _STICK_X_PS2					-1.2f
//#define _STICK_Y_PS						3.07f
//#define _STICK_Z_PS2					1.92f
//#endif
//#define _STICK_ROT_X					FMATH_DEG2RAD( -20.0f )

#define _CONSOLE_X_PS					0.0f
#define _CONSOLE_Y_PS					0.0f
#define _CONSOLE_Z_PS					3.72f

#define _CHIP_INSERT_DELTA_Y			0.5f

#define _SHIELD_Z_OFFSET				( 0.0f )		// z offset to shield origin from console origin
#define _SHIELD_Y_OFFSET				( 3.5f )		// y offset to shield origin from console origin
#define _SHIELD_SCALE					( 0.6f )		// final scale of shield

// activation animations
#define _ACTIVATION_TIME				( 3.0f )		// total time of console activation animation, in seconds.
#define _ACTIVATE_RATE					( 1.0f / _ACTIVATION_TIME )	// rate at which unitized console activation animation proceeds, in units-per-second
#define _SHIELD_UNIT_START				( 0.4f )		// unit start time of shield expansion
#define _SHIELD_UNIT_LENGTH				( 0.1f )		// unit end time of shield expansion
#define _SHIELD_UNIT_END				( _SHIELD_UNIT_START + _SHIELD_UNIT_LENGTH )		// unit end time of shield expansion
#define _SHIELD_UNIT_OSC_LENGTH			( 0.5f )		// unit length of post shield expansion oscillation
#define _SHIELD_OSC_RATE				( FMATH_DEG2RAD( 360.0f * 8.0f ) )	// rate at which shield oscillates, in radians per second (sine curve used for oscillate)
#define _SHIELD_OSC_SCALE				( _SHIELD_SCALE * 0.06f )	// magnitude of shield oscillations

#define _GLITCH_USE_START				( _SHIELD_UNIT_END )	// unit start time of controlling animation
#define _GLITCH_USE_BLEND_LENGTH		( 0.1f )		// unit time over which to blend in controlling animation
#define _X_DIST_TO_CONTROL_POSITION		( -0.35f )		// x distance from console origin to control position
#define _Z_DIST_TO_CONTROL_POSITION		( -1.15f )		// z distance from console origin to control position

#define _JUMP_BACK_VEL					( -12.0f )		// velocity at which Glitch jumps away from console after exiting possession

#define _LIGHT_ANIM_RATE				( 0.5f )		// rate at which unit light animation progress, in units per second.
#define _MONITOR_SCROLL_RATE			( 0.1f )		// rate at which monitor texture scrolls, in units per second.
#define _MONITOR_IDLE_SCROLL_RATE		( 0.02f )		// monitor scroll rate when all chips inserted, but console is idle
#define _MONITOR_MIN_EMISSIVE			( -0.6f )		// emissive value when monitor is off
#define _MONITOR_MAX_EMISSIVE			( 1.0f )		// emissive value when monitor is on
#define _MONITOR_EMISSIVE_CHANGE_RATE	( 15.0f )		// rate at which monitor emissive changes, in units per second
#define _MONITOR_TEX_ID					( 11 )
#define _MONITOR_STRIP_TEX_ID			( 12 )
#define _CABINET_LIGHT_TEX_ID			( 13 )
#define _JOYSTICK_LIGHT_TEX_ID			( 14 )
static s32  _nChipLightTexLayerID[] = { 1,2,3,4,5 };
static s32  _nButtonLightTexLayerID[] = { 6,7,8,9,10 };

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEConsoleBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEConsoleBuilder *_pConsoleEntityBuilder = NULL;// Starich - made dynamically allocated instead of static


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsoleBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_CONSOLE, pszEntityType );

	m_pszMilName = NULL;
	m_pszDispenserName = NULL;
	m_pszStickMesh = _DEFAULT_STICK_MESH;
	m_pszConsoleMesh = _DEFAULT_CONSOLE_MESH;
	m_pszBaseSocketName = _DEFAULT_SOCKET_BONE_NAME;
	m_nSocketCount = 0;
	m_nInsertedChipCount = 0;
	m_fChipScale = 1.0f;

	m_bUseOnce = FALSE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsoleBuilder::InterpretTable( void ) {
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "StickMesh" ) ) {

		if( CEntityParser::Interpret_String( &m_pszStickMesh ) ) {
			if( !fclib_stricmp( m_pszStickMesh, "Default" ) ) {
				m_pszStickMesh = _DEFAULT_STICK_MESH;
			} else if( !fclib_stricmp( m_pszStickMesh, "None" ) ) {
				m_pszStickMesh = NULL;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ConsoleMesh" ) ) {

		if( CEntityParser::Interpret_String( &m_pszConsoleMesh ) ) {
			if( !fclib_stricmp( m_pszConsoleMesh, "Default" ) ) {
				m_pszConsoleMesh = _DEFAULT_CONSOLE_MESH;
			} else if( !fclib_stricmp( m_pszConsoleMesh, "None" ) ) {
				m_pszConsoleMesh = NULL;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "EnemyName" ) ) {

		CEntityParser::Interpret_String( &m_pszMilName );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "SocketName" ) ) {

		CEntityParser::Interpret_String( &m_pszBaseSocketName );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "SocketCount" ) ) {

		CEntityParser::Interpret_U32( &m_nSocketCount, 0, 9, TRUE );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "InsertedChipCount" ) ) {

		CEntityParser::Interpret_U32( &m_nInsertedChipCount );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ChipScale" ) ) {

		CEntityParser::Interpret_F32( &m_fChipScale );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Dispenser" ) ) {

		CEntityParser::Interpret_String( &m_pszDispenserName );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "UseOnce" ) )	{
		CEntityParser::Interpret_BOOL( &m_bUseOnce );

		return TRUE;
	}

	// We don't understand this command. Pass it on...

	return CEntityBuilder::InterpretTable();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsoleBuilder::PostInterpretFixup( void ) {
	return TRUE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEConsole
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CEConsole::m_bSystemInitialized;
FMesh_t *CEConsole::m_pErrorMesh;
CFTexInst CEConsole::m_MilLogoTexInst;
FLinkRoot_t CEConsole::m_ConsoleRoot;			// Linked list of consoles
CEConsole::BotInfo_Console_t CEConsole::m_BotInfo_Console;
u32 CEConsole::m_nConsoleClassClientCount = 0;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::InitSystem( void ) {
	FTexDef_t *pTexDef;
	FASSERT( !m_bSystemInitialized );

	// allocate a builder class
	_pConsoleEntityBuilder = fnew CEConsoleBuilder;
	if( !_pConsoleEntityBuilder ) {
		DEVPRINTF( "CEConsole::InitSystem() : Could not allocate the builder object.\n");
		return FALSE;
	}

	m_pErrorMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _ERROR_MESH );

	// Load Mil logo texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _MILLOGO_TEXTURE );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CEConsole::InitSystem(): Could not find texture '%s'. Logo won't be drawn.\n", _MILLOGO_TEXTURE );
	}
	m_MilLogoTexInst.SetTexDef( pTexDef );
	m_MilLogoTexInst.SetFlags( CFTexInst::FLAG_NONE );


	m_bSystemInitialized = TRUE;

	return TRUE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		fdelete( _pConsoleEntityBuilder );
		_pConsoleEntityBuilder = NULL;
		m_pErrorMesh = NULL;
		m_bSystemInitialized = FALSE;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CEConsole::CEConsole() {
	flinklist_InitRoot( &m_ConsoleRoot, FANG_OFFSETOF( CEConsole, m_ConsoleLink ) );
	_ClearDataMembers();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CEConsole::~CEConsole() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::ClassHierarchyLoadSharedResources( void )
{
	FASSERT( m_nConsoleClassClientCount != 0xffffffff );

	++m_nConsoleClassClientCount;

	if( !CEntity::ClassHierarchyLoadSharedResources() )	{
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...

		return FALSE;
	}

	if( m_nConsoleClassClientCount > 1 ) {
		// Resources already loaded...

		return TRUE;
	}

	// Resources not yet loaded...

	FResFrame_t ResFrame = fres_GetFrame();

	if( !fresload_Load( FSNDFX_RESTYPE, _CONSOLE_SOUND_BANK ) ) 
	{
		DEVPRINTF( "CEConsole Load Shared Resources: Could not load sound effect bank '%s'\n", _CONSOLE_SOUND_BANK );
	}

	if( !fgamedata_ReadFileUsingMap( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitWithError;
		return FALSE;
	}

	return TRUE;

_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::ClassHierarchyUnloadSharedResources( void )
{
	FASSERT( m_nConsoleClassClientCount > 0 );

	--m_nConsoleClassClientCount;

	CEntity::ClassHierarchyUnloadSharedResources();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::ClassHierarchyBuild( void ) {
	CMeshEntity *pMeshEntity;
	CFMtx43A Mtx;
	u32 i;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		FASSERT_NOW;
		return FALSE;
	}

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();
	FMemFrame_t MemFrame = fmem_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEConsoleBuilder *pBuilder = (CEConsoleBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Save off bot name string for later...
	if( pBuilder->m_pszMilName ) {
		m_pControlBot = (CBot *)gstring_Main.AddString( pBuilder->m_pszMilName );
		if( m_pControlBot == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Bot not specified.\n" );
			SCRIPT_ERROR( "In console named %s: Bot %s not found", Name(), pBuilder->m_pszMilName );
			goto _ExitWithError;
		}
	} 
	
	if( pBuilder->m_pszDispenserName ) {
		// Save off Dispenser name string for later...
		m_pBotDispenser = (CBotDispenser *)gstring_Main.AddString( pBuilder->m_pszDispenserName );
		if( m_pBotDispenser == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Dispenser not specified.\n" );
			SCRIPT_ERROR( "In console named %s: Dispenser %s not found", Name(), pBuilder->m_pszDispenserName );
			goto _ExitWithError;
		}
	}

	m_fChipScale = pBuilder->m_fChipScale;
	m_nSocketCount = pBuilder->m_nSocketCount;
	m_nInsertedChipCount = pBuilder->m_nInsertedChipCount;
	FMATH_CLAMPMAX( m_nInsertedChipCount, m_nSocketCount );
	m_nDrawChipCount = m_nInsertedChipCount;

	if( pBuilder->m_pszBaseSocketName && m_nSocketCount ) {
		m_pszBaseSocketName = gstring_Main.AddString( pBuilder->m_pszBaseSocketName );
	}

	// Build chips...
	if( m_nSocketCount ) {
		m_pSlotArray = fnew CConsoleSlot[m_nSocketCount];
		if( m_pSlotArray == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Not enough memory to build m_pSlotArray.\n" );
			goto _ExitWithError;
		}

		Mtx.Identity();
		Mtx.Mul( pBuilder->m_fChipScale );

		for( i=0; i<m_nSocketCount; ++i ) {
			pMeshEntity = &m_pSlotArray[i].m_MEChip;

			if( !pMeshEntity->Create( _CHIP_MESH, NULL, "ConChip", &Mtx ) ) {
				DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Could not create m_MEChip.\n" );
				goto _ExitWithError;
			}

			pMeshEntity->SetCollisionFlag( FALSE );
			pMeshEntity->SetLineOfSightFlag( FALSE );
			pMeshEntity->RemoveFromWorld();
		}
	}

	// Build our operator box...
	m_pOperatorBox = fnew CEBox;
	if( m_pOperatorBox == NULL ) {
		DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Not enough memory to build m_pOperatorBox.\n" );
		goto _ExitWithError;
	}

	Mtx.Identity();
	Mtx.m_vPos.Set(
		_DEFAULT_OPERATOR_BOX_MAX_X - 0.5f*_DEFAULT_OPERATOR_BOX_DIM_X,
		_DEFAULT_OPERATOR_BOX_MAX_Y - 0.5f*_DEFAULT_OPERATOR_BOX_DIM_Y,
		_DEFAULT_OPERATOR_BOX_MAX_Z - 0.5f*_DEFAULT_OPERATOR_BOX_DIM_Z
	);

	if( !m_pOperatorBox->Create( _DEFAULT_OPERATOR_BOX_DIM_X, _DEFAULT_OPERATOR_BOX_DIM_Y, _DEFAULT_OPERATOR_BOX_DIM_Z, "ConZone", &Mtx, NULL, 4 ) ) {
		DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Could not create m_pOperatorBox.\n" );
		goto _ExitWithError;
	}

	m_pOperatorBox->ArmTripwire( TRUE );
	m_pOperatorBox->SetTripwireEventMask( TRIPWIRE_EVENTFLAG_INSIDE_EVENT );
	m_pOperatorBox->SetTripwireFilterMask( TRIPWIRE_FILTER_INCLUDE_PLAYER );
	m_pOperatorBox->SetActionable( TRUE );
	m_pOperatorBox->SetActionFunction( _OperatorBoxAction );
	
	m_pOperatorBox->SetRestoreAttach( TRUE ); //have this guy re-attach itself to us when it restores!

	// Build our joystick mesh entity...
	if( pBuilder->m_pszStickMesh ) {
		// Joystick specified...

		m_pMEJoystick = fnew CMeshEntity;
		if( m_pMEJoystick == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Not enough memory to build m_pMEJoystick.\n" );
			goto _ExitWithError;
		}

		Mtx.Identity();
//		Mtx.RotateX( _STICK_ROT_X );
//		Mtx.m_vPos.Set( _STICK_X_PS2, _STICK_Y_PS, _STICK_Z_PS2 );

		if( !m_pMEJoystick->Create( pBuilder->m_pszStickMesh, NULL, "ConStick", &Mtx ) ) {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Could not create m_pMEJoystick.\n" );
			goto _ExitWithError;
		}

		m_pMEJoystick->SetCollisionFlag( FALSE );
		m_pMEJoystick->SetLineOfSightFlag( FALSE );
	}

	// Build our console mesh entity...
	if( pBuilder->m_pszConsoleMesh ) {
		// Console mesh specified...

		m_pMEConsole = fnew CMeshEntity;
		if( m_pMEConsole == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Not enough memory to build m_pMEConsole.\n" );
			goto _ExitWithError;
		}

		Mtx.Identity();
		Mtx.m_vPos.Set( _CONSOLE_X_PS, _CONSOLE_Y_PS, _CONSOLE_Z_PS );

		if( !m_pMEConsole->Create( pBuilder->m_pszConsoleMesh, NULL, "Console", &Mtx ) ) {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild(): Could not create m_pMEConsole.\n" );
			goto _ExitWithError;
		}
	}

	// Success...

	m_pShield = fnew CEShield;
	if( !m_pShield->Create( "ConsoleShield" ) ) {
		DEVPRINTF( "CEConsole::ClassHierarchyBuild():  Error creating shield\n" );
		goto _ExitWithError;
	} else {
		ShieldInit_t shieldInit;

		shieldInit.uShieldFlags			= 0;
		shieldInit.fShieldRechargeTime	= 0.01f;
		shieldInit.fShieldRechargeDelay	= 0.01f;
		shieldInit.fShieldScale			= _SHIELD_SCALE;
//		shieldInit.pArmorProfile		= CDamage::FindArmorProfile( "Infinite" );	// damage not passed to InflictDamage*, so no shield impact effects were generated
		shieldInit.pArmorProfile		= CDamage::FindArmorProfile( "Titan Shield" );

		m_pShield->Init( NULL, &shieldInit, FALSE /*bStartOn*/ );
		m_pShield->RemoveFromWorld();
		m_pShield->SetShieldInvulnerable( TRUE );
	}

	s32 nIndex;
	for( nIndex = 0; nIndex < MAX_CHIPS; nIndex++ ) {
		m_hChipLightTex[ nIndex ] = m_pMEConsole->GetMesh()->GetTexLayerHandle( _nChipLightTexLayerID[ nIndex ] );
		if( m_hChipLightTex[ nIndex ] != FMESH_TEXLAYERHANDLE_INVALID ) {
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hChipLightTex[ nIndex ], 0.0f, 0.0f, 0.0f );
		} else {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild():  Unable to get tex layer handle for Chip Light %d \n", nIndex );
			goto _ExitWithError;

		}

		m_hButtonLightTex[ nIndex ] = m_pMEConsole->GetMesh()->GetTexLayerHandle( _nButtonLightTexLayerID[ nIndex ] );
		if( m_hButtonLightTex[ nIndex ] != FMESH_TEXLAYERHANDLE_INVALID ) {
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hButtonLightTex[ nIndex ], 0.0f, 0.0f, 0.0f );
		} else {
			DEVPRINTF( "CEConsole::ClassHierarchyBuild():  Unable to get tex layer handle for button Light %d \n", nIndex );
			goto _ExitWithError;
		}
	}

	m_hMonitorTex = m_pMEConsole->GetMesh()->GetTexLayerHandle( _MONITOR_TEX_ID );
	if( m_hMonitorTex != FMESH_TEXLAYERHANDLE_INVALID ) {
		m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hMonitorTex, 0.0f, 0.0f, 0.0f );
		m_pMEConsole->GetMesh()->AnimTC_AnimateScroll( m_hMonitorTex, FALSE );
	} else {
		DEVPRINTF( "CEConsole::ClassHierarchyBuild():  Unable to get tex layer handle for Monitor \n" );
		goto _ExitWithError;
	}

	m_hMonitorStripTex = m_pMEConsole->GetMesh()->GetTexLayerHandle( _MONITOR_STRIP_TEX_ID );
	if( m_hMonitorStripTex != FMESH_TEXLAYERHANDLE_INVALID ) {
		m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hMonitorStripTex, 0.0f, 0.0f, 0.0f );
		m_pMEConsole->GetMesh()->AnimTC_AnimateScroll( m_hMonitorStripTex, FALSE );
	} else {
		DEVPRINTF( "CEConsole::ClassHierarchyBuild():  Unable to get tex layer handle for Monitor Strip \n" );
		goto _ExitWithError;
	}

	m_hHeaderTex = m_pMEConsole->GetMesh()->GetTexLayerHandle( _CABINET_LIGHT_TEX_ID );
	if( m_hHeaderTex != FMESH_TEXLAYERHANDLE_INVALID ) {
//		m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hHeaderTex, 0.0f, 0.0f, 0.0f );
	} else {
		DEVPRINTF( "CEConsole::ClassHierarchyBuild():  Unable to get tex layer handle for Cabinet light \n" );
		goto _ExitWithError;
	}

	m_hJoystickTex = m_pMEJoystick->GetMesh()->GetTexLayerHandle( _JOYSTICK_LIGHT_TEX_ID );
	if( m_hJoystickTex != FMESH_TEXLAYERHANDLE_INVALID ) {
		m_pMEJoystick->GetMesh()->LayerEmissive_Set( m_hJoystickTex, 0.0f, 0.0f, 0.0f );
	} else {
		DEVPRINTF( "CEConsole::ClassHierarchyBuild():  Unable to get tex layer handle for joystick light \n" );
		goto _ExitWithError;
	}

	if( pBuilder->m_bUseOnce ) {
		m_nConsoleFlags |= _CONSOLE_FLAG_USE_ONCE;
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::ClassHierarchyDestroy( void ) {
	fdelete( m_pMEConsole );
	fdelete( m_pMEJoystick );
	fdelete( m_pOperatorBox );
	fdelete_array( m_pSlotArray );

	if( m_pShield != NULL ) {
		fdelete( m_pShield );
		m_pShield = NULL;
	}

	_ClearDataMembers();

	CEntity::ClassHierarchyDestroy();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_ClearDataMembers( void ) {
	m_nConsoleFlags = _CONSOLE_FLAG_NONE;
	m_pMEConsole = NULL;
	m_pMEJoystick = NULL;
	m_pOperatorBox = NULL;
	m_nSocketCount = 0;
	m_nDrawChipCount = 0;
	m_nInsertedChipCount = 0;
	m_fUnitInserting = 0.0f;
	m_nConsoleState = CONSOLE_STATE_IDLE;
	m_pSlotArray = NULL;
	m_fChipScale = 1.0f;
	m_pszBaseSocketName = NULL;

	m_EndQuat_WS.Identity();
	m_EndPos_WS.Zero();
	m_fFlyinAngle = 0.0f;
	m_pGlitchBot = NULL;

	m_fUnitCamAnim = 0.0f;
	m_fStartHalfFOV = 0.0f;
	m_fUnitDrawMilLogo = 0.0f;
	m_fPossessSpeed = 1.0f;

	m_pControlBot = NULL;
	m_pBotDispenser = NULL;
	m_pBotDispenserNet = NULL;
	m_fUnitActivateAnim = 0.0f;
	m_fUnitLightAnim = 0.0f;
	m_fMonitorScroll = 0.0f;
	m_fMonitorEmissive = _MONITOR_MIN_EMISSIVE;
	m_pAttractEmitter = NULL;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CEntityBuilder *CEConsole::GetLeafClassBuilder( void ) {
	return _pConsoleEntityBuilder;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::ClassHierarchyResolveEntityPointerFixups( void ) {
	CEntity::ClassHierarchyResolveEntityPointerFixups();

	// Attach our children to us...

	m_pMEConsole->Attach_ToParent_PS( this );

	s32 nBoneIndex = m_pMEConsole->GetMesh()->FindBone( _BOX_BONE_NAME );
	if( nBoneIndex >= 0 ) {
		CFMtx43A *pMtx;
		pMtx = m_pMEConsole->GetMesh()->GetBoneMtxPalette()[ nBoneIndex ];

		if( pMtx ) {
			m_pOperatorBox->Relocate_RotXlatFromUnitMtx_WS( pMtx );
			m_pOperatorBox->Attach_ToParent_WS( this );
		} else {
			DEVPRINTF( "Matrix for bone '%s' not found in console %s.\n", _BOX_BONE_NAME, Name() );
			DEVPRINTF( "Console may not operate correctly.\n" );
			m_pOperatorBox->Attach_ToParent_PS( this );
		}
	} else {
		DEVPRINTF( "'%s' not found as a bone of console %s.\n", _BOX_BONE_NAME, Name() );
		DEVPRINTF( "Console may not operate correctly.\n" );
		SCRIPT_ERROR( "In console named %s: bone %s not found", Name(), _BOX_BONE_NAME );
		m_pOperatorBox->Attach_ToParent_PS( this );
	}

	if( !_AttachJoystickToConsole() ) {
		DEVPRINTF( "'%s' not found as a bone of console %s.\n", _JOYSTICK_BONE_NAME, Name() );
		DEVPRINTF( "Console may not operate correctly.\n" );
		SCRIPT_ERROR( "In console named %s: bone %s not found", Name(), _JOYSTICK_BONE_NAME );
		m_pMEJoystick->Attach_ToParent_PS( this );
	}

	// Build CConsoleSlot::m_SocketMtx for all slots...
	if( !_LocateSlotPositions( m_pszBaseSocketName ) ) {
		// Shut down...
		m_pOperatorBox->ArmTripwire( FALSE );
		m_pOperatorBox->SetActionable( FALSE );
	} else {
		// Successful...

		// Position any pre-inserted chips...

		u32 i;

		for( i=0; i<m_nInsertedChipCount; ++i ) {
			m_pSlotArray[i].m_MEChip.Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &m_pSlotArray[i].m_SocketMtx, m_fChipScale );
			m_pSlotArray[i].m_MEChip.AddToWorld();
		}

		EnableOurWorkBit();
	}

	cchar *pszEnemyName = (cchar *)m_pControlBot;

	if( m_pControlBot ) {
		m_pControlBot = (CBot *)CEntity::Find( pszEnemyName );

		if( m_pControlBot == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyResolveEntityPointerFixups(): Enemy '%s' not found for this console to control.\n", pszEnemyName );
			DEVPRINTF( "CEConsole::ClassHierarchyResolveEntityPointerFixups(): Console will not operate.\n" );
			SCRIPT_ERROR( "In console %s: bot %s not found", Name(), pszEnemyName );
		}

	}

	// Check all the conditions that require a bot to be dispensed and then possessed.
	cchar *pszDispenserName = (cchar *)m_pBotDispenser;

	if( m_pBotDispenser ) {
		BOOL bCanUseDispenser = ( m_pControlBot != NULL );
		CEntity* pBotDispenserEntity = CEntity::Find( pszDispenserName );
		m_pBotDispenser = CAlarmNet::FindDispenser( pBotDispenserEntity );

		if( m_pBotDispenser == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyResolveEntityPointerFixups(): BotDispenser '%s' not found for this console to control.\n", pszDispenserName );
			DEVPRINTF( "CEConsole::ClassHierarchyResolveEntityPointerFixups(): Console will not operate.\n" );
			SCRIPT_ERROR( "In console %s: dispenser %s not found", Name(), pszDispenserName );
			bCanUseDispenser = FALSE;
		}

		// Also find the net that this dispenser belongs to
		m_pBotDispenserNet = CAlarmNet::FindAlarmNet( m_pBotDispenser );
		if( m_pBotDispenserNet == NULL ) {
			DEVPRINTF( "CEConsole::ClassHierarchyResolveEntityPointerFixups(): BotDispenser '%s' alarm net not found for this console to control.\n", pszDispenserName );
			DEVPRINTF( "CEConsole::ClassHierarchyResolveEntityPointerFixups(): Console will not operate.\n" );
			SCRIPT_ERROR( "In console %s: alarm net not found for dispenser %s", Name(), pszDispenserName );
			bCanUseDispenser = FALSE;
		}

		// Now, if we can't use the dispenser, just zero out the control bot, because we won't be
		// able to use this possession console...
		// If we can, then reserve the control bot specifically for our use, because we are selfish, selfish, selfish.
		if( !bCanUseDispenser ) {
			m_pControlBot = NULL;
		} else {
			// Reserve this bot specifically for this dispenser
			m_pControlBot->m_pCanOnlyBeDispensedBy = m_pBotDispenser;
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::ClassHierarchyAddToWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CEntity::ClassHierarchyAddToWorld();

	if( m_pMEConsole ) {
		m_pMEConsole->AddToWorld();
	}

	if( m_pMEJoystick ) {
		m_pMEJoystick->AddToWorld();
	}

	if( m_pOperatorBox ) {
		m_pOperatorBox->AddToWorld();
	}

	for( i=0; i<m_nDrawChipCount; ++i ) {
		m_pSlotArray[i].m_MEChip.AddToWorld();
	}

	if( m_pShield ) {
		m_pShield->AddToWorld();
		CFVec3A vPos;
		vPos.Set( m_MtxToWorld.m_vFront );
		vPos.Mul( _SHIELD_Z_OFFSET );
		vPos.y += _SHIELD_Y_OFFSET;
		vPos.Add( m_MtxToWorld.m_vPos );
		m_pShield->MtxToWorld()->m_vPos.Set( vPos );
		m_pShield->Attach_ToParent_WS( this );
		m_pShield->EnableShield( FALSE );
	}

	if( m_pAttractEmitter == NULL )	{
		m_pAttractEmitter = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Console.pSoundGroupAttractLoop, FALSE, &m_MtxToWorld.m_vPos );
		if( m_pAttractEmitter == NULL )		{
			DEVPRINTF( "Unable to alloc 'n play console attract loop SFX.\n" );
		}
	} else {
		FASSERT_NOW;
	}

	// add this console to linked list of in-world consoles 
	if( !flinklist_IsLinkInList(&m_ConsoleRoot, &m_ConsoleLink ) ) {
		flinklist_AddTail( &m_ConsoleRoot, this );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::ClassHierarchyRemoveFromWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	if( m_pMEConsole ) {
		m_pMEConsole->RemoveFromWorld();
	}

	if( m_pMEJoystick ) {
		m_pMEJoystick->RemoveFromWorld();
	}

	if( m_pOperatorBox ) {
		m_pOperatorBox->RemoveFromWorld();
	}

	for( i=0; i<m_nDrawChipCount; ++i ) {
		m_pSlotArray[i].m_MEChip.RemoveFromWorld();
	}

	if( m_pShield ) {
		m_pShield->RemoveFromWorld();
	}

	if( m_pAttractEmitter ) {
		m_pAttractEmitter->Destroy();
		m_pAttractEmitter = FALSE;
	}

	// remove this console from linked list of in-world consoles 
	if( flinklist_IsLinkInList(&m_ConsoleRoot, &m_ConsoleLink ) )
	{
		flinklist_Remove( &m_ConsoleRoot, this );
	}

	CEntity::ClassHierarchyRemoveFromWorld();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::ClassHierarchyRelocated( void *pIdentifier ) {
	CEntity::ClassHierarchyRelocated( pIdentifier );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::ClassHierarchyWork( void ) {


#if !FANG_PRODUCTION_BUILD
#if _GIMME_CHIPS
	static BOOL _bDone = FALSE;

	if( !_bDone ) {
		_bDone = TRUE;
		CBotGlitch *pBotGlitch;
		pBotGlitch = (CBotGlitch*) Player_aPlayer[0].m_pEntityOrig;
		pBotGlitch->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo = 10;
	}
#endif
#endif

	CEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

//	if( m_pOperatorBox ) m_pOperatorBox->Draw();
	_UpdateLights();

	switch( m_nConsoleState ) {
	case CONSOLE_STATE_IDLE:
		break;

	case CONSOLE_STATE_CHIP_FLYING:
		_Work_ChipFlying();
		break;

	case CONSOLE_STATE_CHIP_INSERTING:
		_Work_ChipInserting();
		break;

	case CONSOLE_STATE_START_ACTIVATION:
		_Work_StartActivation();
		break;

	case CONSOLE_STATE_ACTIVATING:
		_Work_ActivateAnimation();
		break;

	case CONSOLE_STATE_START_POSSESSION:
		m_nConsoleState = CONSOLE_STATE_FADE_OUT_WITH_LOGO;
		break;

	case CONSOLE_STATE_FADE_OUT_WITH_LOGO:
		_Work_FadeOutWithLogo();
		break;

	case CONSOLE_STATE_FADE_IN_BEHIND_BOT:
		_Work_FadeInBehindBot();
		break;

	case CONSOLE_STATE_CONTROLLING_BOT:
		_Work_ControllingBot();
		break;

	case CONSOLE_STATE_CHECKPOINT_RESTORE:
		_Work_CheckpointRestore();
		break;

	default:
		FASSERT_NOW;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_ChipFlying( void )
{
	CFMtx43A Mtx;
	CFVec3A StartPos_WS, OffsetY, OffsetZ;
	CFQuatA StartQuat, InterpQuat;
	CConsoleSlot *pSlot = (CConsoleSlot *)&m_pSlotArray[m_nInsertedChipCount];

	m_fUnitInserting += FLoop_fPreviousLoopSecs * 1.5f;
	if( m_fUnitInserting >= 1.0f ) {
		m_fUnitInserting = 0.0f;
		CFSoundGroup::PlaySound( m_BotInfo_Console.pSoundGroupInsertingChip );
		m_nConsoleState = CONSOLE_STATE_CHIP_INSERTING;
	} else {
		m_fFlyinAngle = m_fUnitInserting * FMATH_2PI;

		CFMtx43A::m_RotY.SetRotationY( m_fFlyinAngle );

		// Compute starting position and quaternion...

		OffsetY.Mul( FXfm_pView->m_MtxR.m_vUp, 0.3f );
		OffsetZ.Mul( FXfm_pView->m_MtxR.m_vFront, 3.0f );
		StartPos_WS.Add( OffsetY, OffsetZ ).Add( FXfm_pView->m_MtxR.m_vPos );

		StartQuat.BuildQuat( FXfm_pView->m_MtxR );

		// Compute current interpolated quaternion...
		InterpQuat.ReceiveSlerpOf( m_fUnitInserting, StartQuat, m_EndQuat_WS );

		// Compute the current matrix...
		InterpQuat.BuildMtx( Mtx );
		Mtx.m_vPos.Lerp( m_fUnitInserting, StartPos_WS, m_EndPos_WS );

		Mtx.Mul( CFMtx43A::m_RotY );

		pSlot->m_MEChip.Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &Mtx, m_fChipScale );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_ChipInserting( void )
{
	CFMtx43A Mtx;
	CConsoleSlot *pSlot = (CConsoleSlot *)&m_pSlotArray[m_nInsertedChipCount];

	m_fUnitInserting += FLoop_fPreviousLoopSecs * 2.0f;
	if( m_fUnitInserting >= 1.0f ) {
		m_fUnitInserting = 0.0f;
		++m_nInsertedChipCount;

		if( m_nInsertedChipCount == m_nSocketCount ) {

			// We have all the chips.

			if (m_pGlitchBot &&
				!m_pGlitchBot->IsDeadOrDying() &&
				m_pGlitchBot->IsInWorld() &&
				!m_pGlitchBot->IsMarkedForWorldRemove() )
			{
				m_nConsoleState = CONSOLE_STATE_START_ACTIVATION;
			}
		} else {
			// wait for more chips
			m_nConsoleState = CONSOLE_STATE_IDLE;
		}
	} else {
		Mtx.m_vRight = pSlot->m_SocketMtx.m_vRight;
		Mtx.m_vUp = pSlot->m_SocketMtx.m_vUp;
		Mtx.m_vFront = pSlot->m_SocketMtx.m_vFront;

		Mtx.m_vPos.Mul( Mtx.m_vUp, (1.0f - m_fUnitInserting) * _CHIP_INSERT_DELTA_Y ).Add( pSlot->m_SocketMtx.m_vPos );

		pSlot->m_MEChip.Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &Mtx, m_fChipScale );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_StartActivation( void ) {
	m_pShield->EnableShield( TRUE );
	m_pShield->SetShieldScale(0.0f);
	m_pShield->SetOwner( m_pGlitchBot );
	m_pShield->Work();
	m_fUnitActivateAnim = 0.0f;
	m_pGlitchBot->SetInvincible( TRUE );

	m_pGlitchBot->UpdateUseConsoleTap( 0.0f );
	m_pGlitchBot->SetUseConsoleControl( 0.0f );
	m_pGlitchBot->ImmobilizeBot();
	m_pGlitchBot->m_pWorldMesh->SetCollisionFlag( FALSE );

	_TriggerPossession();
	_StartPossessionEffect();

	CFSoundGroup::PlaySound( m_BotInfo_Console.pSoundGroupActivating );
	m_nConsoleState = CONSOLE_STATE_ACTIVATING;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_ActivateAnimation( void ) {

	// update master control unit value for activation animations
	m_fUnitActivateAnim += _ACTIVATE_RATE * FLoop_fPreviousLoopSecs;
	if( m_fUnitActivateAnim >= 1.0f ) {
		m_fUnitActivateAnim = 1.0f;
		m_nConsoleState = CONSOLE_STATE_START_POSSESSION;
	}

	// animate shield activation
	if( m_fUnitActivateAnim <= _SHIELD_UNIT_START ) {
		// pre-shield-expansion
		m_pShield->SetShieldScale( 0.0f );
		m_nConsoleFlags &= ~_CONSOLE_FLAG_PLAYED_SHIELD;
	} else if( m_fUnitActivateAnim > _SHIELD_UNIT_START && m_fUnitActivateAnim <= _SHIELD_UNIT_END ) {
		// shield expanding
		
		if( !(m_nConsoleFlags & _CONSOLE_FLAG_PLAYED_SHIELD) ) {
			CFSoundGroup::PlaySound( m_BotInfo_Console.pSoundGroupShieldUp );
			m_nConsoleFlags |= _CONSOLE_FLAG_PLAYED_SHIELD;
		}
		m_pShield->SetShieldScale( fmath_Div( m_fUnitActivateAnim - _SHIELD_UNIT_START, _SHIELD_UNIT_LENGTH ) * _SHIELD_SCALE );
	}  else if( m_fUnitActivateAnim > _SHIELD_UNIT_END && m_fUnitActivateAnim <= _SHIELD_UNIT_END + _SHIELD_UNIT_OSC_LENGTH ) {
		// post-expansion oscillate
		f32 fUnitOsc;
		fUnitOsc = fmath_Div( m_fUnitActivateAnim - _SHIELD_UNIT_END, _SHIELD_UNIT_OSC_LENGTH );
		m_pShield->SetShieldScale( _SHIELD_SCALE + fmath_Sin( fUnitOsc * _SHIELD_OSC_RATE ) * _SHIELD_OSC_SCALE * ( 1.0f - fUnitOsc ) );
	}
	m_pShield->Work();

	// move glitch to the console-operating position over time
	if( m_fUnitActivateAnim < _GLITCH_USE_START ) {
		m_pGlitchBot->SetUseConsoleControl( 0.0f );
		m_pGlitchBot->UpdateUseConsoleTap( 0.0f );

		CFVec3A vTargetPoint, vDelta, vUnitDir;
		_ComputeOperationPosition( vTargetPoint );

		f32 fMag;
		vDelta.Set( vTargetPoint );
		vDelta.Sub( m_pGlitchBot->MtxToWorld()->m_vPos );
		fMag = vUnitDir.SafeUnitAndMag( vDelta );
		if( fMag > 0.0f ) {
			CFMtx43A mOr;
			CFQuatA qStart, qEnd, qSlerp;
			f32 fUnitTime = fmath_Div( m_fUnitActivateAnim, _GLITCH_USE_START );

			// compute interpolated quat
			qStart.BuildQuat( *m_pGlitchBot->MtxToWorld() );
			qEnd.BuildQuat( m_MtxToWorld );
			qSlerp.ReceiveSlerpOf( fUnitTime, qStart, qEnd );

			// build interpolated matrix and reposition glitch
			qSlerp.BuildMtx( mOr );
			vDelta.Mul( fUnitTime );
			mOr.m_vPos.Set( vDelta );
			mOr.m_vPos.Add( m_pGlitchBot->MtxToWorld()->m_vPos );
			m_pGlitchBot->Relocate_RotXlatFromUnitMtx_WS(&mOr);
		}

	} else if( m_fUnitActivateAnim < _GLITCH_USE_START + _GLITCH_USE_BLEND_LENGTH ) {
		// blend in console-using animation
		f32 fBlend;
		fBlend = fmath_Div( m_fUnitActivateAnim - _GLITCH_USE_START, _GLITCH_USE_BLEND_LENGTH );
		m_pGlitchBot->SetUseConsoleControl( fBlend );

		_PlaceGlitchAtConsole();
	} else {
		// animate glitch using console
		m_pGlitchBot->DeltaUseConsoleTap();
		_AnimateJoystick();

		_PlaceGlitchAtConsole();
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_FadeOutWithLogo( void ) {
	// Get the player index
	s32 nPlayerIndex = m_pGlitchBot->m_nPossessionPlayerIndex;
	CPlayer *pPlayer = &Player_aPlayer[ nPlayerIndex ];

	m_pShield->Work();
	m_pGlitchBot->DeltaUseConsoleTap();
	_PlaceGlitchAtConsole();
	_AnimateJoystick();

	// Update camera animation along tether cable...
	m_fUnitCamAnim += FLoop_fPreviousLoopSecs * 0.3f * m_fPossessSpeed;
	if( m_fUnitCamAnim >= 1.0f ) {
		// camera animation complete

		// If we are possessing a dispensed bot, make sure that the bot construction is done
		if( m_pBotDispenser ) {
			if( m_pControlBot->IsUnderConstruction() ) {
				// The bot is still under construction, so bail out waiting here...
				return;
			}
		}

		fcamera_GetCameraByIndex(nPlayerIndex)->SetFOV( m_fStartHalfFOV );
		fcamera_GetCameraByIndex(nPlayerIndex)->StunCamera( -1.0f );

		// Transfer over to possessed Mil...
		FASSERT( m_pGlitchBot->m_nPossessionPlayerIndex >= 0 );
		m_pControlBot->Possess( m_pGlitchBot->m_nPossessionPlayerIndex, m_pControlBot->GetArmorModifier() );
		m_pControlBot->SetPossessedByConsole( TRUE );

		// Remove Mil logo...
		pPlayer->SetDrawOverlayFunction( NULL );
		FMATH_CLEARBITMASK( m_nConsoleFlags, _CONSOLE_FLAG_DRAWING_MIL_LOGO );

		// Start reducing view white saturation...
		CFColorRGBA StartColorRGBA, EndColorRGBA;
		StartColorRGBA.OpaqueWhite();
		EndColorRGBA.TransparentWhite();

		pPlayer->StartViewFade( &StartColorRGBA, &EndColorRGBA, 0.5f );

		// We're done with this possess mode, so go to next mode...
		m_nConsoleState = CONSOLE_STATE_FADE_IN_BEHIND_BOT;
		return;
	}

	// Handle Mil logo fade-in...
	if( !(m_nConsoleFlags & _CONSOLE_FLAG_DRAWING_MIL_LOGO) ) {
		if( m_fUnitCamAnim > 0.25f ) {
			// Start Mil logo fade-in...

			FMATH_SETBITMASK( m_nConsoleFlags, _CONSOLE_FLAG_DRAWING_MIL_LOGO );
			m_fUnitDrawMilLogo = 0.0f;
			pPlayer->SetDrawOverlayFunction( _DrawMilLogoOverlay, this );
			CFSoundGroup::PlaySound( m_BotInfo_Console.pSoundGroupPossessing );
		}
	} else {
		if( m_fUnitDrawMilLogo < 1.0f ) {
			// Animate Mil logo fade-in...

			m_fUnitDrawMilLogo += FLoop_fPreviousLoopSecs * 0.75f * m_fPossessSpeed;
			FMATH_CLAMPMAX( m_fUnitDrawMilLogo, 1.0f );
		}
	}

	// Handle view white-saturation...
	if( !(m_nConsoleFlags & _CONSOLE_FLAG_WHITESAT_VIEW) ) {
		if( m_fUnitCamAnim > 0.35f ) {
			// Start the white-saturation...

			FMATH_SETBITMASK( m_nConsoleFlags, _CONSOLE_FLAG_WHITESAT_VIEW );

			CFColorRGBA StartColorRGBA, EndColorRGBA;
			StartColorRGBA.TransparentWhite();
			EndColorRGBA.OpaqueWhite();

			pPlayer->StartViewFade( &StartColorRGBA, &EndColorRGBA, fmath_Inv( m_fPossessSpeed ) );
		}
	}

	// Compute new camera FOV...
	if( m_fUnitDrawMilLogo > 0.0f ) {
		f32 fUnitInterp, fHalfFOV;
		fUnitInterp = fmath_UnitLinearToSCurve( m_fUnitDrawMilLogo );
		fUnitInterp *= fUnitInterp;
		fHalfFOV = FMATH_FPOT( fUnitInterp, m_fStartHalfFOV, FMATH_DEG2RAD( 70.0f ) );

		fcamera_GetCameraByIndex(nPlayerIndex)->SetFOV( fHalfFOV );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_FadeInBehindBot( void ) {
	CPlayer *pPlayer = &Player_aPlayer[ m_pControlBot->m_nPossessionPlayerIndex ];

	m_pShield->Work();
	m_pGlitchBot->DeltaUseConsoleTap();
	_PlaceGlitchAtConsole();
	_AnimateJoystick();

	// Wait for white-sat to be completely removed...
	if( pPlayer->GetViewFadeUnitProgress() == 1.0f ) {
		// White-sat is gone...

		FMATH_CLEARBITMASK( m_nConsoleFlags, _CONSOLE_FLAG_WHITESAT_VIEW );
		pPlayer->StopViewFade();

		pPlayer->EnableEntityControl();

		fcamera_GetCameraByIndex(m_pControlBot->m_nPossessionPlayerIndex)->SetFOV( m_fStartHalfFOV );

		// Enable pause screen and weapon select...
		CPauseScreen::SetEnabled( TRUE );

		// TODO: We need to make sure the right HUD is enabled for this player
		CHud2::GetCurrentHud()->SetWSEnable( TRUE );

		if( m_pBotDispenser ) {
			// We Just finished the construction, so turn off the alarm
			m_pBotDispenserNet->TurnOff( TRUE );
		}

#if 0
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if( pDamageForm )
		{
			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile = CDamage::FindDamageProfile( "SentinelCannon" );
			pDamageForm->m_Epicenter_WS = m_pControlBot->MtxToWorld()->m_vPos;
			pDamageForm->m_Damager.nDamagerPlayerIndex = 0;
			pDamageForm->m_Damager.pEntity = NULL;
			pDamageForm->m_pDamageeEntity = m_pControlBot;
			CDamage::SubmitDamageForm( pDamageForm );
		}
#endif

		m_nConsoleState = CONSOLE_STATE_CONTROLLING_BOT;
		return;
	}

	f32 fHalfFOV;

	fHalfFOV = fmath_UnitLinearToSCurve( pPlayer->GetViewFadeUnitProgress() );
	fHalfFOV = FMATH_FPOT( fHalfFOV, FMATH_DEG2RAD( 50.0f ), m_fStartHalfFOV );

	fcamera_GetCameraByIndex(m_pControlBot->m_nPossessionPlayerIndex)->SetFOV( fHalfFOV );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_ControllingBot( void ) {

	m_pShield->Work();
	_PlaceGlitchAtConsole();
	_AnimateJoystick();

	// animate Glitch moving console joystick in proportion to movement speed of controlled bot
	f32 fDelta = fmath_RandomFloatRange( 0.0f, 0.2f ) + 3.0f * m_pControlBot->m_fClampedNormSpeedXZ_WS;
	FMATH_CLAMPMAX( fDelta, 1.0f );
	m_pGlitchBot->DeltaUseConsoleTap( fDelta * FLoop_fPreviousLoopSecs );

	aibrainman_SetLODOverrideForOneWork( m_pGlitchBot->AIBrain() );	// ensure Glitch work runs for one frame to update animation

	if( m_pControlBot && m_pControlBot->m_nPossessionPlayerIndex < 0 ) {
		// Player has been sent back to his original bot...
		m_pShield->DropShield( 0.0f );
		m_pShield->EnableShield( FALSE );
		m_pGlitchBot->SetInvincible( FALSE );
		m_pGlitchBot->SetUseConsoleControl( 0.0f );
		m_pGlitchBot->m_pWorldMesh->SetCollisionFlag( TRUE );
		m_pGlitchBot->MobilizeBot();
		CFVec3A vJumpVel;
		vJumpVel.Set( m_MtxToWorld.m_vFront );
		vJumpVel.y = -1.0f;
		vJumpVel.Mul( _JUMP_BACK_VEL );	// jump back vel should be negative
		m_pGlitchBot->StartVelocityJump( &vJumpVel );
		_AttachJoystickToConsole();
		CFSoundGroup::PlaySound( m_BotInfo_Console.pSoundGroupDeactivating );
		if( m_nConsoleFlags & _CONSOLE_FLAG_USE_ONCE ) {
			// if this is a one-use console, mark it as unusable.
			m_nConsoleFlags |= _CONSOLE_FLAG_USED_IT_ONCE;
		}
		m_nConsoleState = CONSOLE_STATE_IDLE;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_Work_CheckpointRestore( void ) {
	u32 nIndex;

	switch( m_nRestoreState ) {
		case CONSOLE_STATE_FADE_IN_BEHIND_BOT:
		case CONSOLE_STATE_CONTROLLING_BOT:
			// in these states, console was possessing a bot at time of checkpoint save,
			// so restore to possessing state.
			m_pShield->EnableShield( TRUE );
			m_pShield->SetShieldScale( _SHIELD_SCALE );
			m_pShield->Work();
			_PlaceGlitchAtConsole();
			m_pMEJoystick->AddToWorld();	// hmm, why do I have to do this?
			m_pGlitchBot->m_pWorldMesh->SetCollisionFlag( FALSE );
			m_pGlitchBot->ImmobilizeBot();
			m_pGlitchBot->SetUseConsoleControl( 1.0f );
			m_nConsoleState = CONSOLE_STATE_CONTROLLING_BOT;
			break;

		case CONSOLE_STATE_CHIP_FLYING:
		case CONSOLE_STATE_CHIP_INSERTING:
			// chip was on its way into socket and chip inventory was already debited
			++m_nInsertedChipCount;
			m_pShield->DropShield( 0.0f );
			m_pShield->EnableShield( FALSE );
			m_nConsoleState = CONSOLE_STATE_IDLE;

		default:
			m_pShield->DropShield( 0.0f );
			m_pShield->EnableShield( FALSE );
			m_nConsoleState = CONSOLE_STATE_IDLE;
			break;
	}

	// put any chips that were flying or inserting at save time fully into their sockets
	for( nIndex = 0; nIndex < m_nInsertedChipCount; nIndex++ ) {
		m_pSlotArray[ nIndex ].m_MEChip.Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &m_pSlotArray[ nIndex ].m_SocketMtx, m_fChipScale );
		m_pSlotArray[ nIndex ].m_MEChip.AddToWorld();
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::InOperationArea( CBotGlitch *pBotGlitch ) {
	// See if Glitch is facing the right way...
	f32 fDot;
	fDot = pBotGlitch->m_MountUnitFrontXZ_WS.Dot( m_pOperatorBox->MtxToWorld()->m_vFront );
	if( fDot < 0.7f ) {
		return FALSE;
	}

	// Find out if he's inside the operator's box...
	if( m_pOperatorBox->IsPointWithinBox( pBotGlitch->MtxToWorld()->m_vPos ) == FALSE ) {
		return FALSE;
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::_OperatorBoxAction( CEntity *pActionerEntity, CEntity *pOperatorBoxEntity ) {
	FASSERT( pOperatorBoxEntity->TypeBits() & ENTITY_BIT_BOX );

	if( !(pActionerEntity->TypeBits() & ENTITY_BIT_BOTGLITCH) ) {
		// Not Glitch...
		return FALSE;
	}
	CBotGlitch *pBotGlitch = (CBotGlitch *)pActionerEntity;

	CEConsole *pConsole = (CEConsole *)pOperatorBoxEntity->GetParent();
	FASSERT( pConsole->TypeBits() & ENTITY_BIT_CONSOLE );
	if( !(pConsole->TypeBits() & ENTITY_BIT_CONSOLE) ) {
		return FALSE;
	}

	if( pConsole->m_nConsoleState != CONSOLE_STATE_IDLE ) {
		// Console not ready to receive chips or control bot
		return FALSE;
	}

	if( !pConsole->InOperationArea( pBotGlitch ) ) {
		return FALSE;
	}

	if( !pConsole->m_pControlBot ) {
		SCRIPT_ERROR( "Console Control Bot Handle is NULL!" );
		return FALSE;
	}

	if( pConsole->m_pControlBot->IsDeadOrDying() && pConsole->m_pBotDispenser == NULL ) {
		return FALSE;
	}

	if( pConsole->WasUsedOnce() ) {
		return FALSE;
	}

	if( !pConsole->m_pBotDispenser && !pConsole->m_pControlBot->IsInWorld() ) 	{
		SCRIPT_ERROR( "Console Control Bot %s isn't in the world.", pConsole->m_pControlBot->Name() ? pConsole->m_pControlBot->Name() : "NONAME");
		return FALSE;
	}


	if( pConsole->m_pBotDispenser ) {
		// Ensure our autobot is in the alarm net pool...
		u32 i;
		BOOL bFound = FALSE;
		for( i=0; i<CAlarmNet::m_uAutoBotPoolSize; i++ ) {
			if( CAlarmNet::m_papAutoBotPool[ i ] == pConsole->m_pControlBot ) {
				bFound = TRUE;
				break;
			}
		}
		if( !bFound ) {
			SCRIPT_ERROR( "Dispensable Console Control Bot %s isn't in the AutoBot pool.", pConsole->m_pControlBot->Name() ? pConsole->m_pControlBot->Name() : "NONAME");
			return FALSE;
		}

		// Next, ensure our bot is NOT in the world
		if ( pConsole->m_pControlBot->IsInWorld() ) {
			SCRIPT_ERROR( "Dispensed Console Control Bot %s IS in the world.", pConsole->m_pControlBot->Name() ? pConsole->m_pControlBot->Name() : "NONAME");
			return FALSE; //don't accept the chip
		}
	}

	// Glitch is standing in the operator region and facing the right way...

	// See if we already have put all the chips in...
	if( pConsole->m_nInsertedChipCount >= pConsole->m_nSocketCount ) {
		// We have already put in all the chips, just do the possession stuff!
		pConsole->m_pGlitchBot = pBotGlitch;
		pConsole->m_nConsoleState = CONSOLE_STATE_START_ACTIVATION;
		return TRUE;
	}

	// See if he has a chip...
	if( pBotGlitch->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo <= 0 ) {
		// He doesn't have a chip...
#if 1
		return FALSE;
#else
		//xxxxxxxxxxxxxxxxx
		pBotGlitch->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo = 10;
#endif
	}

	// If we are here, then we are inserting a chip.

	CFMtx43A Mtx;
	CFVec3A OffsetY, OffsetZ;
	CConsoleSlot *pSlot = &pConsole->m_pSlotArray[ pConsole->m_nInsertedChipCount ];

	--pBotGlitch->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo;
	FASSERT( pBotGlitch->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo >= 0 );
	FMATH_CLAMPMIN( pBotGlitch->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo, 0 );

	pConsole->m_fUnitInserting = 0.0f;

	// Compute ending position and quaternion...
	pConsole->m_EndPos_WS.Mul( pSlot->m_SocketMtx.m_vUp, _CHIP_INSERT_DELTA_Y ).Add( pSlot->m_SocketMtx.m_vPos );
	pConsole->m_EndQuat_WS.BuildQuat( pSlot->m_SocketMtx );

	pConsole->m_fFlyinAngle = 0.0f;
	pConsole->m_pGlitchBot = pBotGlitch;

	Mtx = FXfm_pView->m_MtxR;
	OffsetY.Mul( Mtx.m_vUp, 0.3f );
	OffsetZ.Mul( Mtx.m_vFront, 3.0f );
	Mtx.m_vPos.Add( OffsetY ).Add( OffsetZ );

	pSlot->m_MEChip.Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &Mtx, pConsole->m_fChipScale );
	pSlot->m_MEChip.AddToWorld();

	++pConsole->m_nDrawChipCount;
	FASSERT( pConsole->m_nDrawChipCount <= pConsole->m_nSocketCount );
	FMATH_CLAMPMAX( pConsole->m_nDrawChipCount, pConsole->m_nSocketCount );

	CFSoundGroup::PlaySound( m_BotInfo_Console.pSoundGroupFlyingChip );
	pConsole->m_nConsoleState = CONSOLE_STATE_CHIP_FLYING;

	return TRUE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_TriggerPossession( void ) {

	FASSERT( m_pControlBot );
	if ( !m_pBotDispenser ) { // This is the standard possession technology
		if( !m_pControlBot->IsInWorld() ) {
			m_pControlBot->AddToWorld();
		}
	} else {
		// This console operates in tandem with a bot dispenser.  
		// Fire up the alarm net the bot dispenser belongs to and get the possessed bot to be built!
		m_pBotDispenserNet->TurnOn( NULL, NULL );
	}

	m_pGlitchBot->ImmobilizeBot();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_StartPossessionEffect( void ) {
	// Start camera fly mode...
	m_fUnitCamAnim = 0.0f;
	m_fUnitDrawMilLogo = 0.0f;
	FMATH_CLEARBITMASK( m_nConsoleFlags, _CONSOLE_FLAG_WHITESAT_VIEW | _CONSOLE_FLAG_DRAWING_MIL_LOGO );

	// Get the player index and his camera
	s32 nPlayerIndex = m_pGlitchBot->m_nPossessionPlayerIndex;

	// Save off original camera FOV...
	CFCamera* pGCam = fcamera_GetCameraByIndex(PLAYER_CAM(nPlayerIndex));
	pGCam->GetFOV( &m_fStartHalfFOV );

	// Don't allow pause screen or weapon select here...
	CPauseScreen::SetEnabled( FALSE );
	CHud2::GetHudForPlayer(nPlayerIndex)->SetWSEnable( FALSE );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_DrawMilLogoOverlay( u32 nPlayerIndex, void *pUser ) {
	CEConsole *pConsole = (CEConsole *)pUser;
	if( pConsole == NULL ) {
		return;
	}

	FASSERT( pConsole->m_pGlitchBot->m_nPossessionPlayerIndex >= 0 );
	CPlayer *pPlayer = &Player_aPlayer[ pConsole->m_pGlitchBot->m_nPossessionPlayerIndex ];

	FViewport_t *pViewport = pPlayer->m_pViewportOrtho3D;

	CFColorRGBA ColorRGBA;
	FDrawVtx_t aVtx[4];
	f32 fZ, fTexOffset;
	u32 i;

	ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pConsole->m_fUnitDrawMilLogo * pConsole->m_fUnitDrawMilLogo * 0.75f );

	for( i=0; i<4; ++i ) {
		aVtx[i].ColorRGBA = ColorRGBA;
	}

	fZ = 0.5f * (pViewport->fNearZ + pViewport->fFarZ);

	aVtx[0].Pos_MS.Set( -pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
	aVtx[1].Pos_MS.Set(  pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
	aVtx[2].Pos_MS.Set( -pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );
	aVtx[3].Pos_MS.Set(  pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );

	fTexOffset = FMATH_FPOT( pConsole->m_fUnitDrawMilLogo, 1.4f, 0.8f );

	aVtx[0].ST.Set( 0.5f - fTexOffset, 0.5f - fTexOffset );
	aVtx[1].ST.Set( 0.5f + fTexOffset, 0.5f - fTexOffset );
	aVtx[2].ST.Set( 0.5f - fTexOffset, 0.5f + fTexOffset );
	aVtx[3].ST.Set( 0.5f + fTexOffset, 0.5f + fTexOffset );

	fviewport_SetActive( pViewport );
	CFXfm::InitStack();

	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_SetTexture( &m_MilLogoTexInst );

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );

	frenderer_Pop();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::_LocateSlotPositions( cchar *pszNamePrefix ) {
	u32 i, nPrefixCharCount;
	//	CEntity *pChildEntity;

	FMemFrame_t MemFrame = fmem_GetFrame();

	nPrefixCharCount = fclib_strlen( pszNamePrefix );

	char *pszSlotName = (char *)fmem_Alloc( nPrefixCharCount + 2 );
	if( pszSlotName == NULL ) {
		DEVPRINTF( "CEConsole::_LocateSlotPositions(): Not enough memory to allocate pszSlotName.\n" );
		goto _ExitWithError;
	}

	fclib_strcpy( pszSlotName, pszNamePrefix );
	pszSlotName[nPrefixCharCount + 1] = 0;

	s32 nBoneIndex;
	CFMtx43A *pMtx;

	for( i=0; i<m_nSocketCount; ++i ) {
		pszSlotName[nPrefixCharCount] = '1' + (char)i;

		nBoneIndex = m_pMEConsole->GetMesh()->FindBone(pszSlotName);
		if( nBoneIndex < 0 ) {
			DEVPRINTF( "CEConsole::_LocateSlotPositions(): Socket '%s' not found as a bone of console %s.\n", pszSlotName, Name() );
			DEVPRINTF( "CEConsole::_LocateSlotPositions(): Console will not operate.\n" );
			SCRIPT_ERROR( "In console named %s: socket named %s not found.", Name(), pszSlotName );
			goto _ExitWithError;
		}

		pMtx = m_pMEConsole->GetMesh()->GetBoneMtxPalette()[ nBoneIndex ];
		if( !pMtx ) {
			DEVPRINTF( "CEConsole::_LocateSlotPositions(): Matrix not found for bone '%s' in console %s.\n", pszSlotName, Name() );
			DEVPRINTF( "CEConsole::_LocateSlotPositions(): Console will not operate.\n" );
			goto _ExitWithError;
		}

		m_pSlotArray[i].m_SocketMtx = *pMtx;
	}

	// Success...

	fmem_ReleaseFrame( MemFrame );
	return TRUE;

	// Failure...
_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_UpdateLights( void ) {
	m_fUnitLightAnim += FLoop_fPreviousLoopSecs * _LIGHT_ANIM_RATE;
	if( m_fUnitLightAnim > 1.0f ) {
		m_fUnitLightAnim -= 1.0f;
	}

	switch( m_nConsoleState ) {
	case CONSOLE_STATE_IDLE:
	case CONSOLE_STATE_CHIP_FLYING:
	case CONSOLE_STATE_CHIP_INSERTING:
		// set light(s) for inserted chips on, except for a brief period where each inserted light turns off in sequence
		if( m_fUnitLightAnim < 0.8f ) {
			_SetChipLights( 0, m_nInsertedChipCount - 1, LIGHT_COLOR_GREEN );
			_SetButtonLights( 0, m_nInsertedChipCount - 1, LIGHT_COLOR_GREEN );
		} else {
			// briefly turn off each inserted chip light in sequence
			u32 uLight = (u32)(5.0f * ( m_fUnitLightAnim - 0.8f ) * (f32)m_nSocketCount);
			FMATH_CLAMP( uLight, 0, m_nSocketCount - 1 );

			_SetChipLights( 0, m_nInsertedChipCount - 1, LIGHT_COLOR_GREEN );
			_SetButtonLights( 0, m_nInsertedChipCount - 1, LIGHT_COLOR_GREEN );

			if( uLight <= m_nInsertedChipCount - 1 ) {
				_SetChipLights( uLight, uLight, LIGHT_COLOR_BLACK );
				_SetButtonLights( uLight, uLight, LIGHT_COLOR_BLACK );
			}
		}

		if( m_nInsertedChipCount < m_nSocketCount ) {
			// set light for next socket to insert flashing
			u32 uOn;
			uOn = (u32)(m_fUnitLightAnim * 16.0f);	// flash at 8x rate of m_fUnitLightAnim
			if( uOn & 1 ) {
				_SetChipLights( m_nInsertedChipCount, m_nInsertedChipCount, LIGHT_COLOR_RED );
				_SetButtonLights( m_nInsertedChipCount, m_nInsertedChipCount, LIGHT_COLOR_RED );
			} else {
				_SetChipLights( m_nInsertedChipCount, m_nInsertedChipCount, LIGHT_COLOR_BLACK );
				_SetButtonLights( m_nInsertedChipCount, m_nInsertedChipCount, LIGHT_COLOR_BLACK );
			}

			// set light(s) for uninserted chips off
			_SetChipLights( m_nInsertedChipCount + 1, m_nSocketCount - 1, LIGHT_COLOR_BLACK );
			_SetButtonLights( m_nInsertedChipCount + 1, m_nSocketCount - 1, LIGHT_COLOR_BLACK );
		}

		// let monitor scroll slowly during idle if all chips are inserted
		if( m_nInsertedChipCount == m_nSocketCount ) {
			m_fMonitorScroll += FLoop_fPreviousLoopSecs * _MONITOR_IDLE_SCROLL_RATE;
		}

		// flicker monitor while chip is being inserted
		if( m_nConsoleState == CONSOLE_STATE_CHIP_INSERTING || m_nConsoleState == CONSOLE_STATE_CHIP_FLYING ) {
			if( fmath_RandomChance( 0.2f ) ) {
				_SetMonitor( TRUE );
				_SetMonitorStrip( TRUE );
				m_fMonitorScroll += FLoop_fPreviousLoopSecs * _MONITOR_SCROLL_RATE;
			} else {
				_SetMonitor( FALSE );
				_SetMonitorStrip( FALSE );
			}

			// flash joystick light rapidly while chip is being inserted
			u32 uOn;
			uOn = (u32)(m_fUnitLightAnim * 16.0f);	// flash at 8x rate of m_fUnitLightAnim
			if( uOn & 1 ) {
				_SetJoystickLight( TRUE );
			} else {
				_SetJoystickLight( FALSE );
			}

		} else {
			_SetMonitor( FALSE );
			_SetMonitorStrip( FALSE );

			// flash joystick light slowly while in idle mode
			u32 uOn;
			uOn = (u32)(m_fUnitLightAnim * 4.0f);	// flash at 2x rate of m_fUnitLightAnim
			if( uOn & 1 ) {
				_SetJoystickLight( TRUE );
			} else {
				_SetJoystickLight( FALSE );
			}
		}

		break;

	case CONSOLE_STATE_START_ACTIVATION:
	case CONSOLE_STATE_ACTIVATING:
	case CONSOLE_STATE_START_POSSESSION:
	case CONSOLE_STATE_FADE_OUT_WITH_LOGO:
	case CONSOLE_STATE_FADE_IN_BEHIND_BOT:
	case CONSOLE_STATE_CONTROLLING_BOT:
		{
			u32 nLight;
			LightColor_e eColor = LIGHT_COLOR_GREEN;
			nLight = (u32)(m_fUnitLightAnim * m_nSocketCount * 4.0f);
			if( (nLight / m_nSocketCount) & 1 ) {
				eColor = LIGHT_COLOR_RED;	// alternate button flash cycle red and green. one cycle each color
			}
			nLight = nLight % m_nSocketCount;
			FMATH_CLAMP( nLight, 0, m_nSocketCount - 1 );

			_SetChipLights( 0, m_nSocketCount - 1, LIGHT_COLOR_BLACK );
			_SetButtonLights( 0, m_nSocketCount - 1, LIGHT_COLOR_BLACK );

			_SetChipLights( nLight, nLight, eColor );
			_SetButtonLights( nLight, nLight, eColor );

			m_fMonitorScroll += FLoop_fPreviousLoopSecs * _MONITOR_SCROLL_RATE;

			u32 uOn;
			uOn = (u32)(m_fUnitLightAnim * 16.0f);	// flash at 8x rate of m_fUnitLightAnim
			if( uOn & 1 ) {
				_SetJoystickLight( TRUE );
			} else {
				_SetJoystickLight( FALSE );
			}

			if( m_nConsoleState == CONSOLE_STATE_ACTIVATING && m_fUnitActivateAnim < _SHIELD_UNIT_START ) {
				// cause monitor to flicker to life during start of activation
				if( fmath_RandomChance( fmath_Div( m_fUnitActivateAnim, _SHIELD_UNIT_START ) ) ) {
					_SetMonitor( TRUE );
					_SetMonitorStrip( TRUE );
				} else {
					_SetMonitor( FALSE );
					_SetMonitorStrip( FALSE );
				}
			} else {
				// monitor on constantly at other times while console active
				_SetMonitor( TRUE );
				_SetMonitorStrip( TRUE );
			}

		}
		break;
	}

	_UpdateMonitor();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// zero-based setting of a range of lights
void CEConsole::_SetChipLights( u32 uStart, u32 uEnd, LightColor_e eColor ) {
	u32 uLight;

	if( uStart >= m_nSocketCount ) {
		return;
	}
	FMATH_CLAMPMAX( uEnd, m_nSocketCount - 1 );

	for( uLight = uStart; uLight <= uEnd; uLight++ ) {
		switch( eColor ) {
		case LIGHT_COLOR_GREEN:
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hChipLightTex[ uLight ], -0.2f, 1.0f, -0.2f );
			break;

		case LIGHT_COLOR_RED:
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hChipLightTex[ uLight ], 1.0f, -0.2f, -0.2f );
			break;

		case LIGHT_COLOR_BLACK:
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hChipLightTex[ uLight ], -0.7f, -0.7f, -0.7f );
			break;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// zero-based setting of a range of lights
void CEConsole::_SetButtonLights( u32 uStart, u32 uEnd, LightColor_e eColor ) {
	u32 uLight;

	if( uStart >= m_nSocketCount ) {
		return;
	}
	FMATH_CLAMPMAX( uEnd, m_nSocketCount - 1 );

	for( uLight = uStart; uLight <= uEnd; uLight++ ) {
		switch( eColor ) {
		case LIGHT_COLOR_GREEN:
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hButtonLightTex[ uLight ], -0.2f, 1.0f, -0.2f );
			break;

		case LIGHT_COLOR_RED:
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hButtonLightTex[ uLight ], 1.0f, -0.2f, -0.2f );
			break;

		case LIGHT_COLOR_BLACK:
			m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hButtonLightTex[ uLight ], -0.7f, -0.7f, -0.7f );
			break;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_SetMonitor( BOOL bOn ) {
	if( bOn ) {
		m_nConsoleFlags |= _CONSOLE_FLAG_MONITOR_ON;
	} else {
		m_nConsoleFlags &= ~_CONSOLE_FLAG_MONITOR_ON;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_SetMonitorStrip( BOOL bOn ) {
	if( bOn ) {
		m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hMonitorStripTex, 1.0f, 1.0f, 1.0f );
	} else {
		m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hMonitorStripTex, -0.5f, -0.5f, -0.5f );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_UpdateMonitor( void ) {
	CFVec2 vVec;
	vVec.x = 0.0f;
	vVec.y = m_fMonitorScroll;
	m_pMEConsole->GetMesh()->AnimTC_SetScrollST( m_hMonitorTex, vVec );

	vVec.x = -2.0f * m_fMonitorScroll;
	if( vVec.x < -1.0f ) {
		vVec.x += 1.0f;
	}
	vVec.y = 0.0f;
	m_pMEConsole->GetMesh()->AnimTC_SetScrollST( m_hMonitorStripTex, vVec );

	if( (m_nConsoleFlags & _CONSOLE_FLAG_MONITOR_ON) ) {
		m_fMonitorEmissive += _MONITOR_EMISSIVE_CHANGE_RATE * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_fMonitorEmissive, _MONITOR_MAX_EMISSIVE );
	} else if( !(m_nConsoleFlags & _CONSOLE_FLAG_MONITOR_ON) ) {
		m_fMonitorEmissive -= _MONITOR_EMISSIVE_CHANGE_RATE * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fMonitorEmissive, _MONITOR_MIN_EMISSIVE );
	}
	m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hMonitorTex, m_fMonitorEmissive, m_fMonitorEmissive, m_fMonitorEmissive );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_SetHeader( BOOL bOn ) {
	if( bOn ) {
// uncomment LayerEmissive_Set in ClassHierarchyBuild to use.  LayerEmissive_Set allocates memory.
//		m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hHeaderTex, 1.0f, 1.0f, 1.0f );
	} else {
//		m_pMEConsole->GetMesh()->LayerEmissive_Set( m_hHeaderTex, -1.0f, -1.0f, -1.0f );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_SetJoystickLight( BOOL bOn ) {
	if( bOn ) {
		m_pMEJoystick->GetMesh()->LayerEmissive_Set( m_hJoystickTex, 1.0f, -0.3f, -0.3f );
	} else {
		m_pMEJoystick->GetMesh()->LayerEmissive_Set( m_hJoystickTex, -0.5f, -0.5f, -0.5f );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::_AttachJoystickToConsole( void ) {
	s32 nBoneIndex = m_pMEConsole->GetMesh()->FindBone( _JOYSTICK_BONE_NAME );
	if( nBoneIndex >= 0 ) {
		CFMtx43A *pMtx;
		pMtx = m_pMEConsole->GetMesh()->GetBoneMtxPalette()[ nBoneIndex ];

		if( pMtx ) {
			m_pMEJoystick->Relocate_RotXlatFromUnitMtx_WS( pMtx );
			m_pMEJoystick->Attach_ToParent_WS( this );
			return TRUE;
		} else {
			return FALSE;
		}
	}

	return FALSE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_AnimateJoystick( void ) {
	if( m_pGlitchBot && m_pGlitchBot->GetMesh() ) {
		s32 nHandBoneIndex;
		nHandBoneIndex = m_pGlitchBot->GetMesh()->FindBone( "Secondary_Fire" );
		if( nHandBoneIndex >= 0 ) {
			CFMtx43A *pHandMtx;
			pHandMtx = m_pGlitchBot->GetMesh()->GetBoneMtxPalette()[ nHandBoneIndex ];

			if( pHandMtx && m_pMEConsole && m_pMEConsole->GetMesh() ) {
				s32 nStickBoneIndex = m_pMEConsole->GetMesh()->FindBone( _JOYSTICK_BONE_NAME );
				if( nStickBoneIndex >= 0 ) {
					CFMtx43A *pStickMtx;
					pStickMtx = m_pMEConsole->GetMesh()->GetBoneMtxPalette()[ nStickBoneIndex ];

					if( pStickMtx ) {
						CFVec3A vStickNormal;
						vStickNormal.Set( pHandMtx->m_vPos );
						vStickNormal.Sub( pStickMtx->m_vPos );
						if( vStickNormal.SafeUnitAndMag( vStickNormal ) ) {
							CFMtx43A mStick;
							mStick.m_vUp.Set( vStickNormal );
							mStick.m_vFront.Set( pStickMtx->m_vFront );
							mStick.m_vRight.Cross( mStick.m_vUp, mStick.m_vFront );
							if( mStick.m_vRight.SafeUnitAndMag( mStick.m_vRight ) ) {
								mStick.m_vPos.Set( pStickMtx->m_vPos );
								m_pMEJoystick->Relocate_RotXlatFromUnitMtx_WS( &mStick );
							}
						}
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_ComputeOperationPosition( CFVec3A &rvPos ) {
	CFVec3A vTemp, vDelta;
	rvPos.Set( m_MtxToWorld.m_vFront );
	rvPos.Mul( _Z_DIST_TO_CONTROL_POSITION );
	vTemp.Set( m_MtxToWorld.m_vRight );
	vTemp.Mul( _X_DIST_TO_CONTROL_POSITION );
	rvPos.Add( vTemp );
	rvPos.Add( m_MtxToWorld.m_vPos );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::_PlaceGlitchAtConsole( void ) {
	CFMtx43A mOr;
	CFVec3A vTargetPoint;
	_ComputeOperationPosition( vTargetPoint );
	mOr.Set( m_MtxToWorld );
	mOr.m_vPos.Set( vTargetPoint );
	m_pGlitchBot->Relocate_RotXlatFromUnitMtx_WS( &mOr );
	m_pGlitchBot->m_pWorldMesh->SetCollisionFlag( FALSE );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	u32 i;

	FASSERT( IsCreated() );

	if( m_pMEConsole ) {
		m_pMEConsole->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pMEJoystick ) {
		m_pMEJoystick->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pOperatorBox ) {
		m_pOperatorBox->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	for( i=0; i<m_nDrawChipCount; ++i ) {
		m_pSlotArray[i].m_MEChip.AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// static function to draw on-screen console messages.  Called by CBotGlitch::DrawText()
void CEConsole::DrawText( CBotGlitch *pBotGlitch  )
{
	CEConsole *pConsole;

	cwchar* wszFormat;
	if( CPlayer::m_nPlayerCount > 1 ) {
		wszFormat = L"~f9~C92929299~w0~ac~o1%ls";
	} else {
		wszFormat = L"~f1~C92929299~w0~ac~s1.00%ls";
	}

	// step through linked list of in-world consoles and print appropriate on-screen messages for pBotGlitch
	pConsole = (CEConsole*) flinklist_GetHead( &m_ConsoleRoot );
	while( pConsole ) {
		FASSERT( pConsole->TypeBits() & ENTITY_BIT_CONSOLE );

		if( pConsole->IsIdle() ) {
			if( pConsole->InOperationArea( pBotGlitch ) ) {
				u32 uRemainingChips;
				uRemainingChips = pConsole->NumSockets() - pConsole->NumInsertedChips();
				FASSERT( uRemainingChips <= pConsole->NumSockets() );	// detect unsigned overflow
				FMATH_CLAMPMAX( uRemainingChips, pConsole->NumSockets() );
				if( uRemainingChips == 0 ) {
					if( pConsole->WasUsedOnce() ||
						((pConsole->m_pControlBot == NULL || pConsole->m_pControlBot->IsDeadOrDying()) && pConsole->m_pBotDispenser == NULL) ) {
						ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_CONSOLE_OUT_OF_ORDER ] );
					} else {
						ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_USE_CONSOLE ] );
					}
				} else if( uRemainingChips == 1 ) {
					ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_ONE_CHIP_REQUIRED ] );
				} else {
					wchar wszTemp[160];
					_snwprintf( wszTemp, 160, Game_apwszPhrases[ GAMEPHRASE_CHIPS_REQUIRED ], uRemainingChips );
					ftext_Printf( 0.5f, 0.58f, wszFormat, wszTemp );
				}

				if( uRemainingChips > 0 ) {
					if( pBotGlitch->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo > 0 ) {
						ftext_Printf( 0.5f, 0.62f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_INSERT_CHIP ] );
					} else {
						ftext_Printf( 0.5f, 0.62f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_NO_CHIPS_TO_INSERT ] );
					}
				}
			}
		}

		pConsole = (CEConsole*) flinklist_GetNext( &m_ConsoleRoot, pConsole );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CEConsole::CheckpointSave( void ) {
	// Save parent class data...
	CEntity::CheckpointSave();

	// Save specific class data. Order must match load order below!
	CFCheckPoint::SaveData( m_nConsoleFlags );

	CFCheckPoint::SaveData( m_nDrawChipCount );
	CFCheckPoint::SaveData( m_nInsertedChipCount );
	CFCheckPoint::SaveData( m_fUnitInserting );	
	CFCheckPoint::SaveData( m_nConsoleState );
	CFCheckPoint::SaveData( m_EndQuat_WS );
	CFCheckPoint::SaveData( m_EndPos_WS );
	CFCheckPoint::SaveData( m_fFlyinAngle );

	return TRUE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEConsole::CheckpointRestore( void ) {
	CEntity::CheckpointRestore();

	// Reload specific variables here...
	CFCheckPoint::LoadData( m_nConsoleFlags );

	CFCheckPoint::LoadData( m_nDrawChipCount );
	CFCheckPoint::LoadData( m_nInsertedChipCount );
	CFCheckPoint::LoadData( m_fUnitInserting );	
	CFCheckPoint::LoadData( (u32&)m_nConsoleState );
	CFCheckPoint::LoadData( m_EndQuat_WS );
	CFCheckPoint::LoadData( m_EndPos_WS );
	CFCheckPoint::LoadData( m_fFlyinAngle );

	m_nRestoreState = m_nConsoleState;
	m_nConsoleState = CONSOLE_STATE_CHECKPOINT_RESTORE;
}


//-----------------------------------------------------------------------------
const FGameData_TableEntry_t CEConsole::m_aBotInfoVocab_Console[] = {
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAttractLoop
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupShieldUp
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFlyingChip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupInsertingChip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupActivating
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupDeactivating
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupPossessing

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CEConsole::m_aGameDataMap[] =
{
	"Console",
	m_aBotInfoVocab_Console,
	sizeof(m_BotInfo_Console),
	(void *)&m_BotInfo_Console,

	NULL
};
