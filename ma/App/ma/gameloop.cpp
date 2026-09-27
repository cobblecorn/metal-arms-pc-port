//////////////////////////////////////////////////////////////////////////////////////
// gameloop.cpp - 
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
// 01/22/02 Starich     Created.
// 06/14/02	Link		Added LaserBeam system.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "gameloop.h"
#include "fresload.h"
#include "fworld.h"
#if FANG_PLATFORM_WIN
	#include "dx/fdx8vid.h"
	#include "win/screenshot.h"
#endif
#include "fcpu.h"
#include "ftext.h"
#include "ffile.h"
#include "floop.h"
#include "fpadio.h"
#include "fpad.h"
#if defined(MA_PC_INPUT)
	#include "pc_input.h"
#endif
#include "fmath.h"
#include "sas_user.h"
#include "user_albert.h"
#include "user_john.h"
#include "user_mike.h"
#include "user_pat.h"
#include "user_steve.h"
#include "level.h"
#include "game.h"
#include "fboxfilter.h"
#include "frenderer.h"
#include "ftimer.h"
#include "gamepad.h"
#include "botgrunt.h"
#include "botpred.h"
#include "bottitan.h"
#include "botzom.h"
#include "site_BotWeapon.h"
#include "fdraw.h"
#include "FScriptSystem.h"
#include "ItemRepository.h"
#include "menutypes.h"
#include "PauseScreen.h"
#include "Hud2.h"
#include "gamecam.h"
#include "Workable.h"
#include "tracer.h"
#include "potmark.h"
#include "BotFx.h"
#include "reticle.h"
#include "player.h"
#include "PSPool.h"
#include "FXStreamer.h"
#include "guid.h"
#include "entity.h"
#include "meshentity.h"
#include "Door.h"
#include "ESwitch.h"
#include "letterbox.h"
#include "gstring.h"
#include "fgamedata.h"
#include "picklevel.h"
#include "launcher.h"
#include "e3menu.h"
#include "wpr_system.h"
#include "wpr_levelcomplete.h"
#include "wpr_languageselect.h"
#include "wpr_startupoptions.h"
#include "ai/aibuilder.h"
#include "MAScriptTypes.h"
#include "TalkSystem2.h"
#include "BarterSystem.h"
#include "weapons.h"
#include "eproj.h"
#include "eproj_saw.h"
#include "eproj_grenade.h"
#include "LaserBeam.h"
#include "MuzzleFlash.h"
//#include "BlinkShell.h"
#include "BlinkSpeed.h"
#include "BlinkGlow.h"
#include "bot.h"
#include "flamer.h"
#include "tether.h"
#include "econsole.h"
#include "EParticle.h"
#include "gamesave.h"
#include "zipline.h"
#include "ProTrack.h"
#include "ShadyAnim.h"
#include "Weapon_Chaingun.h"
#include "vehiclerat.h"
#include "vehiclesentinel.h"
#include "FXShockwave.h"
#include "splat.h"
#include "EShield.h"
#include "damage.h"
#include "fxempblast.h"
#include "fperf.h"
#include "botslosh.h"
#include "Weapon_magmabomb.h"
#include "weapon_quadlaser.h"
#include "weapon_recruiter.h"
#include "sloshtank.h"
#include "fmovie2.h"
#include "botprobe.h"
#include "fxmagmabomb.h"
#include "ebotfire.h"
#include "vehicleloader.h"
#include "botpart.h"
#include "botswarmer.h"
#include "botEliteGuard.h"
#include "botkrunk.h"
#include "botAAGun.h"
#include "weapon_staff.h"
#include "eparticlepool.h"
#include "EDetPackDrop.h"
#include "botjumper.h"
#include "explosion.h"
#include "ftextmon.h"
#include "collectable.h"
#include "botminer.h"
#include "LightPool.h"
#include "sound.h"
#include "botscout.h"
#include "botmortar.h"
#include "botmozer.h"
#include "botglitch.h"
#include "botcorrosive.h"
#include "FXMeshBuild.h"
#include "botscientist.h"
#include "mg_holdyourground.h"
#include "msgbox.h"
#include "botsnarq.h"
#include "edebris.h"
#include "difficulty.h"


//====================
// private definitions

#define _FRAMERATE_BOXFILTER_DEPTH	20

#if FANG_PLATFORM_GC
#define _GC_SAVEGAME_BANNER_FILENAME 	"ma_bannerp"
#define _GC_SAVEGAME_ICON_FILENAME		"ma_iconp"
#define _GC_APPLICATION_NAME			"Metal Arms"
#endif

typedef BOOL _InitSystemFcn_t( void );
typedef void _UninitSystemFcn_t( void );

typedef struct {
	_InitSystemFcn_t *pFcnInitSystem;
	_UninitSystemFcn_t *pFcnUninitSystem;
#if !FANG_PRODUCTION_BUILD
	cchar* pszSysName;
#endif
} _SystemFunctions_t;

// Starich - these macros will take the module name strings out in production mode saving us precious memory :-)
#if !FANG_PRODUCTION_BUILD
	// pass in a string with no quotes and this macro will replace it with the string in quotes with a comma at the end
	// ex. _NAME_TO_STRING( mike )		->		"mike",
	#define _NAME_TO_STRING(NAME)		#NAME,
#else
	// will take a string with no quotes and replace it with nothing
	#define _NAME_TO_STRING(NAME)
#endif



//=================
// public variables

Gameloop_Platform_e Gameloop_nPlatform;

BOOL Gameloop_bSwapEnabled = TRUE;
BOOL Gameloop_bDrawEnabled = TRUE;
BOOL Gameloop_bOverrideFrameTime = TRUE;

BOOL Gameloop_bDrawHUD = TRUE;
BOOL Gameloop_bDrawDebugInfo = TRUE;
BOOL Gameloop_bInstallAudio = TRUE;

//BOOL Gameloop_bDrawDebugInfo = FALSE;
Gameloop_ExitDestination_e Gameloop_ExitDestination = GAMELOOP_EXIT_DESTINATION_NONE;

//==================
// private variables

static GameloopInitParm_t _InitParms;
// resource vars
static FResFrame_t _ResFrame;
static FMemFrame_t _MemFrame;

static Gameloop_AnimPlaybackRates_e _nAnimPlaybackMode = GAMELOOP_DEFAULT_ANIM_PLAYBACK;
static BOOL8 _bDrawBoundingSphere = GAMELOOP_DEFAULT_VIEW_BOUNDS;
static BOOL8 _bDisplayFPS = GAMELOOP_DEFAULT_SHOW_FPS;
static BOOL8 _bDisplayScreenSafeArea = GAMELOOP_DEFAULT_DRAW_SCREEN_SAFE_AREA;
static BOOL8 _bSwitchFunctions;

static GameloopFcn_t *_pFcnWork;		// Work function (NULL=none)
static GameloopFcn_t *_pFcnDraw;		// Draw function (NULL=none)
static GameloopFcn_t *_pFcnInit;		// Init fucntion (NULL=none)
static GameloopFcn_t *_pFcnNewWork;		// New work function (NULL=none)
static GameloopFcn_t *_pFcnNewDraw;		// New draw function (NULL=none)
static GameloopFcn_t *_pFcnNewInit;		// New init fucntion (NULL=none)

static BOOL _bPlatformResetDisabled = FALSE;
static BOOL _bExitGameloop = FALSE;

static BOOL _bIdleTimerOn;
static f32 _fIdleTimer;


#if !FANG_PRODUCTION_BUILD
	// screen save area box vars & defs
	#define _NUM_SCREEN_SAFE_VTXS	5
	#define _SCREEN_SAFE_AREA_R		0.75f
	#define _SCREEN_SAFE_AREA_G		0.75f
	#define _SCREEN_SAFE_AREA_B		0.85f
	#define _SCREEN_SAFE_AREA_A		0.80f
	#define _SCREEN_AREA_PERCENT_LOST_PER_SIDE	0.075f
	#define _SCREEN_AREA_PERCENT_USABLE	( 1.0f - (2.0f * _SCREEN_AREA_PERCENT_LOST_PER_SIDE) )
	static FDrawVtx_t _aScreenSafeArea[_NUM_SCREEN_SAFE_VTXS];
	static FViewport_t *_pViewportOrtho3D = NULL;
	static void _SetupScreenSafeAreaVtxs();
	static void _DrawScreenSafeArea();

	// controls display vars & defs
	#define _DRAW_CONTROLS		TRUE
	#define _CONTROL_INTENSITY	( 185.0f / 255.0f )
	#define _CONTROL_RADIUS		( 40.0f )
	#define _CONTROL_ALPHA		( 0.75f )
#endif


// Put game systems in this array which ***DO NOT*** require the following initialization at System INIT time:
// 1) the language the game is going to run in has been determined.
// 2) Sound banks that must be loaded
// 3) Debris groups that must be loaded -- Because debris groups require sounds also
// 4) The language specific game Phrase table
static const _SystemFunctions_t _aSystemFunctionArray[] = {
	CGString::InitSystem,				CGString::UninitSystem,					_NAME_TO_STRING( GString )
	guid_InitSystem,					guid_UninitSystem,						_NAME_TO_STRING( guid )
	CLightPool::InitSystem,				CLightPool::UninitSystem,				_NAME_TO_STRING( LightPool )
	sound_InitSystem,					sound_UninitSystem,						_NAME_TO_STRING( Sound )
	CDamage::InitSystem,				CDamage::UninitSystem,					_NAME_TO_STRING( Damage )
	CGColl::InitSystem,					CGColl::UninitSystem,					_NAME_TO_STRING( GColl )
	CBotPartPool::InitSystem,			CBotPartPool::UninitSystem,				_NAME_TO_STRING( BotPartPool )
	CEntity::InitSystem,				CEntity::UninitSystem,					_NAME_TO_STRING( Entity )
	CMeshEntity::InitSystem,			CMeshEntity::UninitSystem,				_NAME_TO_STRING( MeshEntity )
	CDoorEntity::InitSystem,			CDoorEntity::UninitSystem,				_NAME_TO_STRING( DoorEntity )
	CESwitch::InitSystem,				CESwitch::UninitSystem,					_NAME_TO_STRING( ESwitch )
	CEParticle::InitSystem,				CEParticle::UninitSystem,				_NAME_TO_STRING( EParticle )
	eparticlepool_InitSystem,			eparticlepool_UninitSystem,				_NAME_TO_STRING( eparticlepool )// must happen after CEParticle
	CEProjPool::InitSystem,				CEProjPool::UninitSystem,				_NAME_TO_STRING( EProjPool )
    pspool_InitSystem,					pspool_UninitSystem,					_NAME_TO_STRING( pspool )
	tracer_InitSystem,					tracer_UninitSystem,					_NAME_TO_STRING( tracer )
//	potmark_InitSystem,					potmark_UninitSystem,					_NAME_TO_STRING( potmark )
	CFScriptSystem::InitSystem,			CFScriptSystem::UninitSystem,			_NAME_TO_STRING( FScriptSystem )
	CWorkable::InitSystem,				CWorkable::UninitSystem,				_NAME_TO_STRING( Workable )
	CMAScriptTypes::InitSystem,			CMAScriptTypes::UninitSystem,			_NAME_TO_STRING( MAScriptTypes )
	gamepad_InitSystem,					gamepad_UninitSystem,					_NAME_TO_STRING( gamepad )
	CPlayer::InitSystem,				CPlayer::UninitSystem,					_NAME_TO_STRING( Player )
	CExplosion2::InitSystem,			CExplosion2::UninitSystem,				_NAME_TO_STRING( Explosion2 ) // must happen before game_InitSystem - NKM
	gamecam_InitSystem,					gamecam_UninitSystem,					_NAME_TO_STRING( gamecam )// must happen before the level or game modules, MIKE
	level_InitSystem,					level_UninitSystem,						_NAME_TO_STRING( level )
	game_InitSystem,					game_UninitSystem,						_NAME_TO_STRING( game )
	CBotPowerupFx::InitSystem,			CBotPowerupFx::UninitSystem,			_NAME_TO_STRING( BotPowerupFx )
	CEZipLine::InitSystem,				CEZipLine::UninitSystem,				_NAME_TO_STRING( EZipLine )
	CFXStreamerEmitter::InitSystem,		CFXStreamerEmitter::UninitSystem,		_NAME_TO_STRING( FXStreamerEmitter )
	CMuzzleFlash::InitSystem,			CMuzzleFlash::UninitSystem,				_NAME_TO_STRING( MuzzleFlash )
	letterbox_InitSystem,				letterbox_UninitSystem,					_NAME_TO_STRING( letterbox )
	picklevel_InitSystem,				picklevel_UninitSystem,					_NAME_TO_STRING( picklevel )// must happen after level_InitSystem, MIKE
	launcher_InitSystem,				launcher_UninitSystem,					_NAME_TO_STRING( launcher )// must happen after level_InitSystem, MIKE
	e3menu_InitSystem,					e3menu_UninitSystem,					_NAME_TO_STRING( e3menu )// must happen after level_InitSystem, MIKE
	CAIBuilder::InitSystem,				CAIBuilder::UninitSystem,				_NAME_TO_STRING( AIBuilder )
	CTalkSystem2::InitSystem,			CTalkSystem2::UninitSystem,				_NAME_TO_STRING( TalkSystem2 )
	CShadyAnim::InitSystem,				CShadyAnim::UninitSystem,				_NAME_TO_STRING( ShadyAnim )
	bartersystem_InitSystem,			bartersystem_UninitSystem,				_NAME_TO_STRING( bartersystem )
	CWeapon::InitSystem,				CWeapon::UninitSystem,					_NAME_TO_STRING( Weapon )
	CWeaponFlamer::InitSystem,			CWeaponFlamer::UninitSystem,			_NAME_TO_STRING( WeaponFlamer )
	CEProj_Saw::InitSystem,				CEProj_Saw::UninitSystem,				_NAME_TO_STRING( EProj_Saw )
//	CBlinkShell::InitSystem,			CBlinkShell::UninitSystem,				_NAME_TO_STRING( BlinkShell )
	CLightningBoltGroup::InitSystem,	CLightningBoltGroup::UninitSystem,		_NAME_TO_STRING( LightningBoltGroup )
	CBlinkSpeed::InitSystem,			CBlinkSpeed::UninitSystem,				_NAME_TO_STRING( BlinkSpeed )
	CBlinkGlow::InitSystem,				CBlinkGlow::UninitSystem,				_NAME_TO_STRING( BlinkGlow )
	CFlamer::InitSystem,				CFlamer::UninitSystem,					_NAME_TO_STRING( Flamer )
	CTether::InitSystem,				CTether::UninitSystem,					_NAME_TO_STRING( Tether )
	CEConsole::InitSystem,				CEConsole::UninitSystem,				_NAME_TO_STRING( EConsole )
	checkpoint_InitSystem,				checkpoint_UninitSystem,				_NAME_TO_STRING( CheckPoint )
	CFXMeshBuildPart::InitSystem,		CFXMeshBuildPart::UninitSystem,			_NAME_TO_STRING( FxMeshBuildPart )
	CFXMeshBuildPartMgr::InitSystem,	CFXMeshBuildPartMgr::UninitSystem,		_NAME_TO_STRING( FxMeshBuildPartMgr )
	CBotPred::InitSystem,				CBotPred::UninitSystem,					_NAME_TO_STRING( BotPred )
	CBotGrunt::InitSystem,				CBotGrunt::UninitSystem,				_NAME_TO_STRING( BotGrunt )
	CBotTitan::InitSystem,				CBotTitan::UninitSystem,				_NAME_TO_STRING( BotTitan )
	CBotZom::InitSystem,				CBotZom::UninitSystem,					_NAME_TO_STRING( BotZom )
	CEDebris::InitSystem,				CEDebris::UninitSystem,					_NAME_TO_STRING( EDebris )
	playerprofile_InitSystem,			playerprofile_UninitSystem,				_NAME_TO_STRING( playerprofile )
	CWeaponChaingun::InitSystem,		CWeaponChaingun::UninitSystem,			_NAME_TO_STRING( WeaponChaingun )
	CBotSiteWeapon::InitSystem,			CBotSiteWeapon::UninitSystem,			_NAME_TO_STRING( BotSiteWeapon )
	CVehicle::InitSystem,				CVehicle::UninitSystem,					_NAME_TO_STRING( Vehicle )
	CVehicleRat::InitSystem,			CVehicleRat::UninitSystem,				_NAME_TO_STRING( VehicleRat )
	CVehicleSentinel::InitSystem,		CVehicleSentinel::UninitSystem,			_NAME_TO_STRING( VehicleSentinel )
	CEProj_Grenade::InitSystem,			CEProj_Grenade::UninitSystem,			_NAME_TO_STRING( EProj_Grenade )
	CFXShockwave::InitSystem,			CFXShockwave::UninitSystem,				_NAME_TO_STRING( FXShockwave )
//	CSplat::InitSystem,					CSplat::UninitSystem,					_NAME_TO_STRING( Splat )
	CEShield::InitSystem,				CEShield::UninitSystem,					_NAME_TO_STRING( EShield )
	CFXEMPBlast::InitSystem,			CFXEMPBlast::UninitSystem,				_NAME_TO_STRING( FXEMPBlast )
	CBotSlosh::InitSystem,				CBotSlosh::UninitSystem,				_NAME_TO_STRING( BotSlosh )
	CWeaponMagmaBomb::InitSystem,		CWeaponMagmaBomb::UninitSystem,			_NAME_TO_STRING( WeaponMagmaBomb )
	CWeaponQuadLaser::InitSystem,		CWeaponQuadLaser::UninitSystem,			_NAME_TO_STRING( WeaponQuadLaser )
	CFXMagmaBomb::InitSystem,			CFXMagmaBomb::UninitSystem,				_NAME_TO_STRING( FXMagmaBomb )
	CEBotFire::InitSystem,				CEBotFire::UninitSystem,				_NAME_TO_STRING( EBotFire )
	CVehicleLoader::InitSystem,			CVehicleLoader::UninitSystem,			_NAME_TO_STRING( VehicleLoader )
	CBotSwarmer::InitSystem,			CBotSwarmer::UninitSystem,				_NAME_TO_STRING( BotSwarmer )
	CBotEliteGuard::InitSystem,			CBotEliteGuard::UninitSystem,			_NAME_TO_STRING( BotEliteGuard )
	CBotKrunk::InitSystem,				CBotKrunk::UninitSystem,				_NAME_TO_STRING( BotKrunk )
	CBotAAGun::InitSystem,				CBotAAGun::UninitSystem,				_NAME_TO_STRING( BotAAGun )
	CWeaponStaff::InitSystem,			CWeaponStaff::UninitSystem,				_NAME_TO_STRING( WeaponStaff )
	CBotJumper::InitSystem,				CBotJumper::UninitSystem,				_NAME_TO_STRING( BotJumper )
	CBotMiner::InitSystem,				CBotMiner::UninitSystem,				_NAME_TO_STRING( BotMiner )
	CBotScout::InitSystem,				CBotScout::UninitSystem,				_NAME_TO_STRING( BotScout )
	CBotMortar::InitSystem,				CBotMortar::UninitSystem,				_NAME_TO_STRING( BotMortar )
	CBotMozer::InitSystem,				CBotMozer::UninitSystem,				_NAME_TO_STRING( BotMozer )
	CBotCorrosive::InitSystem,			CBotCorrosive::UninitSystem,			_NAME_TO_STRING( BotCorrosive )
	CBotScientist::InitSystem,			CBotScientist::UninitSystem,			_NAME_TO_STRING( BotScientist )
	CMsgBox::InitSystem,				CMsgBox::UninitSystem,					_NAME_TO_STRING( CMsgBox )
	CBotSnarq::InitSystem,				CBotSnarq::UninitSystem,				_NAME_TO_STRING( CBotSnarq )
	mg_holdyourground_InitSystem,		mg_holdyourground_UninitSystem,			_NAME_TO_STRING( Hold Your Ground )
	wpr_languageselect_InitSystem,		wpr_languageselect_UninitSystem,		_NAME_TO_STRING( wpr_languageselect )		
	wpr_startupoptions_InitSystem,		wpr_startupoptions_UninitSystem,		_NAME_TO_STRING( wpr_languageselect )		
	NULL,								NULL
};


// Put game systems in this array which require initializing after
// 1) the language the game is going to run in has been determined.
// 2) Sound banks that must be loaded
// 3) Debris groups that must be loaded
// 4) The language specific game Phrase table
static const _SystemFunctions_t _aLocalizedSystemFunctionArray[] = {
	CItemRepository::InitSystem,		CItemRepository::UninitSystem,			_NAME_TO_STRING( ItemRepository )
	CCollectable::InitSystem,			CCollectable::UninitSystem,				_NAME_TO_STRING( CCollectables ) // MRS, collectibles now rely on ItemRespository::InitSystem
	CBot::InitSystem,					CBot::UninitSystem,						_NAME_TO_STRING( Bot ) 
	CMenuMgr::InitSystem,				CMenuMgr::UninitSystem,					_NAME_TO_STRING( MenuMgr )
	CPauseScreen::InitSystem,			CPauseScreen::UninitSystem,				_NAME_TO_STRING( PauseScreen )
	CHud2::InitSystem,					CHud2::UninitSystem,					_NAME_TO_STRING( Hud2 )
	CReticle::InitSystem,				CReticle::UninitSystem,					_NAME_TO_STRING( Reticle )
	CLaserBeam::InitSystem,				CLaserBeam::UninitSystem,				_NAME_TO_STRING( LaserBeam )
	CWeaponRocket::InitSystem,			CWeaponRocket::UninitSystem,			_NAME_TO_STRING( WeaponRocket )
	CWeaponRivet::InitSystem,			CWeaponRivet::UninitSystem,				_NAME_TO_STRING( WeaponRivet )
	CWeaponLaser::InitSystem,			CWeaponLaser::UninitSystem,				_NAME_TO_STRING( WeaponLaser )
	CWeaponBlaster::InitSystem,			CWeaponBlaster::UninitSystem,			_NAME_TO_STRING( WeaponBlaster )
	CWeaponGren::InitSystem,			CWeaponGren::UninitSystem,				_NAME_TO_STRING( WeaponGren )
	CWeaponTether::InitSystem,			CWeaponTether::UninitSystem,			_NAME_TO_STRING( WeaponTether )
	CWeaponSpew::InitSystem,			CWeaponSpew::UninitSystem,				_NAME_TO_STRING( WeaponSpew )
	CWeaponMortar::InitSystem,			CWeaponMortar::UninitSystem,			_NAME_TO_STRING( WeaponMortar )
	CWeaponRipper::InitSystem,			CWeaponRipper::UninitSystem,			_NAME_TO_STRING( WeaponRipper )
	CWeaponCleaner::InitSystem,			CWeaponCleaner::UninitSystem,			_NAME_TO_STRING( WeaponCleaner )
	CWeaponScope::InitSystem,			CWeaponScope::UninitSystem,				_NAME_TO_STRING( WeaponScope )
	CWeaponEMP::InitSystem,				CWeaponEMP::UninitSystem,				_NAME_TO_STRING( WeaponEMP )
	CWeaponRecruiter::InitSystem,		CWeaponRecruiter::UninitSystem,			_NAME_TO_STRING( WeaponRecruiter )
	CBotProbe::InitSystem,				CBotProbe::UninitSystem,				_NAME_TO_STRING( BotProbe )
	CBotGlitch::InitSystem,				CBotGlitch::UninitSystem,				_NAME_TO_STRING( BotGlitch )
	CWeaponWrench::InitSystem,			CWeaponWrench::UninitSystem,			_NAME_TO_STRING( WeaponWrench )
	wpr_levelcomplete_InitSystem,		wpr_levelcomplete_UninitSystem,			_NAME_TO_STRING( wpr_levelcomplete )		
	wpr_system_InitSystem,				wpr_system_UninitSystem,				_NAME_TO_STRING( wpr_system )  // must happen very last, so that the uninit is called first
	NULL,								NULL
};


//===================
// private prototypes

static BOOL _GameInit( void *pParameter );
static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter );
static BOOL _GameMain( BOOL bExitRequest, void *pParameter );

#if FANG_PLATFORM_WIN
	static BOOL _IsEscKeyHit( void );
	static BOOL _ShouldAppMinimize( void );
#else if FANG_PLATFORM_GC
	static void _CheckForGCReset( void );
	static BOOL _bResetButtonDown = FALSE;
#endif

static BOOL _InitGameSystems( void );
static void _UninitGameSystems( void );

static BOOL _WorldOnlyCreateShapeArray( cchar *pszWorldResName, const void *pFixupOffsetBase,
	const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount ) {
	pszWorldResName;
	pFixupOffsetBase;
	pShapeInitArray;
	nShapeInitCount;
	return TRUE;
}




//=================
// public functions

// Returns TRUE if the gameloop started ok.
// pParms cannot be NULL.
BOOL gameloop_Start( GameloopInitParm_t *pParms ) {
	if( !fang_IsStartedUp() ) {
		DEVPRINTF( "Gameloop_Start(): fang must be started before calling.\n" );
		return FALSE;
	}

	if( !pParms ) {
		DEVPRINTF( "Gameloop_Start(): pParms must not be NULL.\n" );
		return FALSE;
	}

	_InitParms = *pParms;
	_ResFrame = NULL;
	_MemFrame = NULL;
	_nAnimPlaybackMode = _InitParms.nAnimPlaybackRate;
	_bDrawBoundingSphere = _InitParms.bViewBounds;
	_bDisplayFPS = _InitParms.bShowFPS;
	_bDisplayScreenSafeArea = _InitParms.bDrawScreenSafeArea;

#if !FANG_PRODUCTION_BUILD
	_pViewportOrtho3D = NULL;
#endif

	_pFcnWork = NULL;
	_pFcnDraw = NULL;
	_pFcnInit = NULL;
	_bSwitchFunctions = FALSE;
	_pFcnNewWork = NULL;
	_pFcnNewDraw = NULL;
	_pFcnNewInit = NULL;

	_bExitGameloop = FALSE;

	_bIdleTimerOn = TRUE;
	_fIdleTimer = 0.0f;

	// Install the gameloop...
	return floop_InstallGameloop( _GameInit, _GameMain, _GameTerm, &_InitParms, pParms->fTargetFPS, 15.0f );
}

void gameloop_End( void ) {
	if( floop_IsGameloopRunning() ) {
		floop_UninstallGameloop();
	}
}

// Enables or disables the drawing of bounding spheres.
void gameloop_SetViewBounds( BOOL bEnable ) {
	_bDrawBoundingSphere = bEnable;	
}

// Returns the current bounding sphere drawing mode.
BOOL gameloop_GetViewBounds( void ) {
	return _bDrawBoundingSphere;
}

BOOL gameloop_GetPlayerDeath( void ) {
	return _InitParms.bPlayerDeath;
}

void gameloop_SetPlayerDeath( BOOL bEnable ) {
	_InitParms.bPlayerDeath = bEnable;
}

BOOL gameloop_GetDebugAI( void ) {
	return _InitParms.bDebugAI;
}

void gameloop_SetDebugAI( BOOL bEnable ) {
	_InitParms.bDebugAI = bEnable;
}

BOOL gameloop_GetShowEntityInfo( void ) {
	return _InitParms.bShowEntityInfo;
}

void gameloop_SetShowEntityInfo( BOOL bEnable ) {
	_InitParms.bShowEntityInfo = bEnable;
}

u32 gameloop_GetLegServoLevel( void ) {
	return _InitParms.nLegServoLevel;
}

u32 gameloop_GetArmServoLevel( void ) {
	return _InitParms.nArmServoLevel;
}

void gameloop_SetLegServoLevel( u32 nUpgradeLevel ) {
	_InitParms.nLegServoLevel = nUpgradeLevel;
}

void gameloop_SetArmServoLevel( u32 nUpgradeLevel ) {
	_InitParms.nArmServoLevel = nUpgradeLevel;
}

void gameloop_ShowFPS( BOOL bShow ) {
	_bDisplayFPS = bShow;
}

BOOL gameloop_GetShowFPS( void ) {
	return _bDisplayFPS;
}

void gameloop_ShowScreenSafeArea( BOOL bShow ) {
	_bDisplayScreenSafeArea = bShow;
}

BOOL gameloop_GetShowScreenSafeArea( void ) {
	return _bDisplayScreenSafeArea;
}

void gameloop_SetAnimPlaybackRate( Gameloop_AnimPlaybackRates_e nRate ) {
	FASSERT( nRate < GAMELOOP_ANIM_PLAYBACK_COUNT );
	_nAnimPlaybackMode = nRate;

}

Gameloop_AnimPlaybackRates_e gameloop_GetAnimPlaybackRate( void ) {
	return _nAnimPlaybackMode;
}

void gameloop_SetLoopHandlers( GameloopFcn_t *pFcnWork, GameloopFcn_t *pFcnDraw, GameloopFcn_t *pFcnInit ) {

	// schedule a function switch to take place at the top of the next gameloop
	_bSwitchFunctions = TRUE;
	_pFcnNewWork = pFcnWork;
	_pFcnNewDraw = pFcnDraw;
	_pFcnNewInit = pFcnInit;
}

void gameloop_GetLoopHandlers( GameloopFcn_t **ppFcnWork, GameloopFcn_t **ppFcnDraw, GameloopFcn_t **ppFcnInit ) {
	if( ppFcnWork ) {
		*ppFcnWork = _pFcnWork;
	}
	if( ppFcnDraw ) {
		*ppFcnDraw = _pFcnDraw;
	}
	if( ppFcnInit ) {
		*ppFcnInit = _pFcnInit;
	}
}

void gameloop_GetBackgroundColor( CFColorRGB &rRGB ) {
	rRGB = _InitParms.BGColorRGB;
}

void gameloop_ScheduleExit( void ) {
	_bExitGameloop = TRUE;
}

void gameloop_EnableOverlayDrawElements( BOOL bEnableHUD, BOOL bEnableDebugInfo ) {
	Gameloop_bDrawHUD = bEnableHUD;
	Gameloop_bDrawDebugInfo = bEnableDebugInfo;

	// turn off on a few one time debug items
	CFScriptSystem::SetMonitorsOn( Gameloop_bDrawDebugInfo );
}

cchar *gameloop_GetSkipLevelName( void ) {
	return _InitParms.pszInputFilename;
}

BOOL gameloop_GetSkipLevelUseMissionData( void ) {
	return _InitParms.bLoadRegisteredMission;
}

u8 gameloop_GetQuickLaunchCampaignPlayers( void ) {
	return _InitParms.nQuickLaunchCampaignPlayers;
}

void gameloop_ResetIdleTimer() {
	_fIdleTimer = 0.0f;
}
	
void gameloop_EnableIdleTimer( BOOL bEnable ) {
	_bIdleTimerOn = bEnable;
}

//==================
// private functions

static BOOL _GameInit( void *pParameter ) {
	BOOL bInitReturnValue = FALSE;
	GameloopInitParm_t *pParm = (GameloopInitParm_t *)pParameter;

	// Grab memory frames...
	_ResFrame = fres_GetFrame();
	_MemFrame = fmem_GetFrame();

	// If we were demo launched, let the launcher know
	Launcher_bDemoLaunched = pParm->bDemoLaunched;

	#if FANG_PLATFORM_WIN
		if( !FCpu_IntelInfo.CPUID.CPUID1.bSIMDTechnology ) {
			DEVPRINTF( "Gameloop.cpp: This game requires a CPU that supports Intel's SIMD instruction set\n." );
			goto _ExitGameInitWithError;
		}

		if( !FCpu_IntelInfo.bSIMDSupportedByOS ) {
			DEVPRINTF( "Gameloop.cpp: Update your OS so that it supports the CPU's SIMD instruction set\n." );
			goto _ExitGameInitWithError;
		}

		if( pParm->bSkipLevelSelect && !pParm->pszInputFilename ) {
			DEVPRINTF( "You must select a world file before you can run.\n" );
			goto _ExitGameInitWithError;
		}		
	#endif

	if( !ffile_IsInMasterMode() ) {
		DEVPRINTF( "You must select a valid master file before you can run.\n" );
 		goto _ExitGameInitWithError;
	}

	// Create the render window...
	if( !fvid_CreateWindow( &pParm->VidWin ) ) {
		DEVPRINTF( "Gameloop.cpp: Could not use the current video mode.\nTry selecting a different video mode.\n" );
		goto _ExitGameInitWithError;
	}

	#if FANG_PLATFORM_WIN
		Gameloop_nPlatform = pParm->nPlatform;
	#elif FANG_PLATFORM_XB
		Gameloop_nPlatform = GAMELOOP_PLATFORM_XB;
	#elif FANG_PLATFORM_GC
		Gameloop_nPlatform = GAMELOOP_PLATFORM_GC;
	#elif FANG_PLATFORM_PS2								//CPS 4.7.03
		Gameloop_nPlatform = GAMELOOP_PLATFORM_PS2;		//CPS 4.7.03
	#else
		#error <Undefined Platform>
	#endif

	floop_EnableGovernor( pParm->bGovernFrameRate );

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStart( "Bootup Loading" );
#endif

	DEVPRINTF( "******** LOAD MARKER - START OF BOOTUP***********.\n" );

	// Init the input system...
	FPadio_Init_t oPadioInit;
	fang_MemZero( &oPadioInit, sizeof( oPadioInit ) );

#if FANG_PLATFORM_WIN
	oPadioInit.DX8ONLY_ohWnd = (u32)fdx8vid_GetWindowHandle();
	oPadioInit.DX8ONLY_ohInstance = (u32)FVid_Win.hInstance;
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
#endif

	if( FPADIO_NO_ERROR != fpadio_Install( &oPadioInit ) ) {
		DEVPRINTF( "Could not install input driver (fpadio).\n" );
		goto _ExitGameInitWithError;
	}
	if( FPAD_NO_ERROR != fpad_Install( 0.5f, 0.1f ) ) {
		DEVPRINTF( "Could not install input driver (fpad).\n" );
		goto _ExitGameInitWithError;
	}

	PROTRACK_INITSYSTEM();	 //couldn't stick in init table because this is a macro that will mean nothing in some builds

	//// Init the text system...
	//
	if( ftext_Install() == FTEXT_ERROR ) {
		fvid_DestroyWindow();
		DEVPRINTF( "Could not install the text system\n." );
		goto _ExitGameInitWithError;
	}

	//
	////

	Gameloop_bInstallAudio = pParm->bInstallAudio;

	// Enable the bounds to be drawn...
	frenderer_DrawBound_Enable( _bDrawBoundingSphere );

	#if FANG_PLATFORM_WIN
		// Hook up the screen shot system (only works in fullscreen mode)...
		screenshot_Init( pParm->pszScreenShotDir, FALSE );
	#endif

	CMenuItem::m_HudCommonTexInst.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2hudall1")) );
	
	// Call the level specific init...

	#if( SAS_ACTIVE_USER == SAS_USER_STEVE )
		bInitReturnValue = 	user_steve_PreworldInit();

	#elif( SAS_ACTIVE_USER == SAS_USER_MIKE )
		bInitReturnValue = 	user_mike_PreworldInit();

	#elif( SAS_ACTIVE_USER == SAS_USER_PAT ) 
		bInitReturnValue = 	user_pat_PreworldInit();

	#elif( SAS_ACTIVE_USER == SAS_USER_JOHN )
		bInitReturnValue = 	user_john_PreworldInit();

	#else( SAS_ACTIVE_USER == SAS_USER_NONE ) 
		// Regular game flow...
		bInitReturnValue = TRUE;

	#endif

	if( !bInitReturnValue ) {
		goto _ExitGameInitWithError;
	}

	#if FANG_PRODUCTION_BUILD
		// Production build...
		floop_SetTargetFramesPerSec( pParm->fTargetFPS );
		floop_EnableFixedLoopTimeMode( FALSE );
		frenderer_DrawBound_Enable( FALSE );
	#endif

	// Initialize all game systems...
	if( !_InitGameSystems() ) {
		// Trouble initializing a game system...
		goto _ExitGameInitWithError;
	}

	if( pParm->bLoadWorldOnly ) {
		FWorldInitShapeArrayCallback_t *pShapeCallback = fworld_SetShapeCreateCallback( _WorldOnlyCreateShapeArray );
		void *pWorld = fresload_Load( FWORLD_RESTYPE, pParm->pszInputFilename );
		fworld_SetShapeCreateCallback( pShapeCallback );
		if( !pWorld ) {
			DEVPRINTF( "World-only load failed for '%s'.\n", pParm->pszInputFilename );
			goto _ExitGameInitWithError;
		}
		DEVPRINTF( "World-only load completed for '%s'.\n", pParm->pszInputFilename );
	#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
	#endif
		gameloop_ScheduleExit();
		return TRUE;
	}

	if( !CDifficulty::LoadData() ) {
		DEVPRINTF( "CDifficulty::LoadData() could not load difficulty data. There will be no difference between difficulty settings.\n" );
	}

#if !LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
	// This is not an E3 or demo version, we can install the fstorage system
	// For external demos or E3 versions, the fstorage system must not be used at all by the game

	// install and update fstorage system
	// (fstorage_ModuleStartup must be called first)
	{
		FStorage_Init_t oStorageInit;
		fang_MemZero( &oStorageInit, sizeof( oStorageInit ) );

	#if FANG_PLATFORM_GC
		// set the icon and data
		oStorageInit.GCONLY_pszIconName = _GC_SAVEGAME_ICON_FILENAME;
		oStorageInit.GCONLY_pszBannerName = _GC_SAVEGAME_BANNER_FILENAME;
		oStorageInit.GCONLY_pszApplicationName = _GC_APPLICATION_NAME;
	#endif

		if( fstorage_Install( &oStorageInit ) != FSTORAGE_ERROR_NONE ) {
			DEVPRINTF( "Error: fstorage_Install() failed.\n" );
			goto _ExitGameInitWithError;
		}

		u32 uConnected;
		u32 uInserted;
		u32 uRemoved;
		fstorage_UpdateDeviceInfos( &uConnected, &uInserted, &uRemoved );
	}
#endif

	#if !FANG_PRODUCTION_BUILD
		_SetupScreenSafeAreaVtxs();
	#endif

	// Call the game init function.
	#if( SAS_ACTIVE_USER == SAS_USER_JOHN ) 
		bInitReturnValue = 	user_john_PreworldInit();
	#endif

	if( !bInitReturnValue ) {
		goto _ExitGameInitWithError;
	}

	// Capture the state of the buttons at app startup time...
	wpr_startupoptions_CaptureAppStartButtons();

	if( !pParm->bSkipLevelSelect ) {
		// load the wrappers so that we can select what level to load
		if( !launcher_EnterMenus( LAUNCHER_FROM_BOOTUP ) ) {
			goto _ExitGameInitWithError;
		}
	} else {
		// load the english fonts and resources and the generic level with the passed in world file
		if( !game_LoadLocalizedPhraseTableAndFonts() ) {
			goto _ExitGameInitWithError;
		}
		if( !game_InitLocalizedResources() ) {
			goto _ExitGameInitWithError;
		}
		if( !launcher_StartGame( LAUNCHER_FROM_QUICK_SKIP ) ) {
			goto _ExitGameInitWithError;	
		}
	}

	#if( SAS_ACTIVE_USER == SAS_USER_STEVE )
		bInitReturnValue = 	user_steve_PrelevelInit();

	#elif( SAS_ACTIVE_USER == SAS_USER_MIKE )
		bInitReturnValue = 	user_mike_PrelevelInit();

	#elif( SAS_ACTIVE_USER == SAS_USER_PAT ) 
		bInitReturnValue = 	user_pat_PrelevelInit();

	#elif( SAS_ACTIVE_USER == SAS_USER_JOHN )
		bInitReturnValue = 	user_john_PrelevelInit();

	#else( SAS_ACTIVE_USER == SAS_USER_NONE ) 
		// Regular game flow...
//		bInitReturnValue = TRUE;
	#endif

	if( !bInitReturnValue ) {
		goto _ExitGameInitWithError;
	}

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif

	DEVPRINTF( "******** LOAD MARKER - END OF BOOTUP***********.\n" );

	// Success!
	return TRUE;

_ExitGameInitWithError:
	// Failure...
	// _GameTerm will be called, don't release the frames yet, the input driver
	// has a background thread that uses mem (YOU WILL CRASH)
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	return FALSE;
}

static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter ) {
	GameloopInitParm_t *pParm = (GameloopInitParm_t *)pParameter;

	game_UnloadLevel();	

	fstorage_Uninstall();

	// Uninitialize all game systems...
	_UninitGameSystems();

	// Call the level specific terminate...
	#if( SAS_ACTIVE_USER == SAS_USER_STEVE )
		user_steve_Terminate();

	#elif( SAS_ACTIVE_USER == SAS_USER_MIKE )
		user_mike_Terminate();

	#elif( SAS_ACTIVE_USER == SAS_USER_PAT ) 
		user_pat_Terminate();

	#elif( SAS_ACTIVE_USER == SAS_USER_JOHN )
		user_john_Terminate();

	#else( SAS_ACTIVE_USER == SAS_USER_NONE ) 
		// Regular game flow...

	#endif	

	faudio_Uninstall();

	// Shutdown the text system...
	ftext_Uninstall();

	PROTRACK_UNINITSYSTEM();	 //couldn't stick in table because this is a macro that will mean nothing in some builds

	// Shuddown the movie system
	fmovie2_Unload();
	fmovie2_Uninstall();

	// Shutdown the input systems...
	fpadio_Uninstall();
	fpad_Uninstall();

	#if FANG_PLATFORM_WIN
		// Shutdown the screen grab system...
		screenshot_Shutdown();	
	#endif

	// Release our memory frames...
	fres_ReleaseFrame( _ResFrame );
	fmem_ReleaseFrame( _MemFrame );

	fvid_DestroyWindow();

	// Call the exit function, if one was set...
	if( pParm->pExitFunc ) {
		pParm->pExitFunc();
	}
}


// Called once per game loop.
static BOOL _GameMain( BOOL bExitRequest, void *pParameter ) {
	GameloopInitParm_t *pParm = (GameloopInitParm_t *)pParameter;

	fperf_TimerReport( FPERF_FRAME, FPERF_TIMER_END );
	fperf_TimerReport( FPERF_FRAME, FPERF_TIMER_START );
	fperf_TimerReport( FPERF_SIM, FPERF_TIMER_START );

	if( bExitRequest || _bExitGameloop ) {
		return FALSE;
	}

#if FANG_PLATFORM_GC
	gameloop_CheckForPlatformReset( FALSE );
#endif

	// handle the idle timer
	if( _bIdleTimerOn ) {
		_fIdleTimer += FLoop_fRealPreviousLoopSecs;

#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
		if ( Launcher_bDemoLaunched ) {
			// handle idle for too long here
			if ( pParm->uTimeoutInterval != 0 && _fIdleTimer * 1000 > pParm->uTimeoutInterval )
			{
				return FALSE;
			}
		}
#endif
	}

	if( _bSwitchFunctions ) {
		// If the previous guy changed these, we want to reset them for the
		//   next guy.
		gameloop_SetDrawEnabled(TRUE);
		gameloop_SetSwapEnabled(TRUE);

		// call the init function
		if( _pFcnNewInit ) {
			if( !_pFcnNewInit() ) {
				return FALSE;
			}
		}
		// save our function ptrs
		_pFcnWork = _pFcnNewWork;
		_pFcnDraw = _pFcnNewDraw;
		_pFcnInit = _pFcnNewInit;
		_bSwitchFunctions = FALSE;			
	}

	#if FANG_PLATFORM_WIN
		// Windows-specific code...

		if( fvid_HasUserClosedWindow() ) {
			return FALSE;
		}

		if( fdx8vid_HaveFocus() ) {
			// Our 3D window has the focus...

			if( _IsEscKeyHit() ) {
				// ESC key has been pressed...
				return FALSE;
			}

			if( _ShouldAppMinimize() ) {
				// Minimize our 3D window...
				if( pParm->pMinFunc ) {
					pParm->pMinFunc();
				}
			}
		} else {
			// allow other windows to get some processing time
			Sleep( 0 );
		}
	#endif

	fmath_RandomInt32();

	if(Gameloop_bOverrideFrameTime)
	{
#if 1
	#if !FANG_PRODUCTION_BUILD
		// Non-production-build code...
		switch( _nAnimPlaybackMode ) {
		case GAMELOOP_ANIM_PLAYBACK_30FPS:
			if( floop_GetTargetFramesPerSec() != 30 || !floop_IsFixedLoopTimeModeEnabled() ) {
				floop_SetTargetFramesPerSec( 30 );
				floop_EnableFixedLoopTimeMode( TRUE );
			}
			break;
		case GAMELOOP_ANIM_PLAYBACK_60FPS:
			if( floop_GetTargetFramesPerSec() != 60 || !floop_IsFixedLoopTimeModeEnabled() ) {
				floop_SetTargetFramesPerSec( 60 );
				floop_EnableFixedLoopTimeMode( TRUE );
			}
			break;
		default:
		case GAMELOOP_ANIM_PLAYBACK_REALTIME:
			if( floop_IsFixedLoopTimeModeEnabled() ) {
				floop_SetTargetFramesPerSec( pParm->fTargetFPS );
				floop_EnableFixedLoopTimeMode( FALSE );
			}
			break;
		}

		// Enable/Disable the bounding sphere...
		frenderer_DrawBound_Enable( _bDrawBoundingSphere );

		#if FANG_PLATFORM_WIN
			// See if we should take a screen shot...
			screenshot_Work();		
		#endif
	#endif
#else
		if( floop_IsFixedLoopTimeModeEnabled() ) {
			floop_SetTargetFramesPerSec( pParm->fTargetFPS );
			floop_EnableFixedLoopTimeMode( FALSE );
		}
#endif
	}

	PROTRACK_WORK();

	// JUSTIN: Add push of view matrix here.
	gamecam_GetActiveCamera()->GetFinalXfm()->InitStackWithView();

	// Do game work...
	PROTRACK_BEGINBLOCK("Work");
	if( _pFcnWork ) {
		if( !_pFcnWork() ) {
			PROTRACK_ENDBLOCK();	//"Work
			return FALSE;
		}
	}
	PROTRACK_ENDBLOCK();	//"Work

	// see if we need to switch control functions...
	if( _bSwitchFunctions ) {
		return TRUE;
	}

	#if( SAS_ACTIVE_USER == SAS_USER_STEVE )
		user_steve_Work();
	#elif( SAS_ACTIVE_USER == SAS_USER_JOHN )
		user_john_Main();
	#endif

	// JUSTIN: Add pop of view matrix here.
	CFXfm::InitStack();

	fperf_TimerReport( FPERF_SIM, FPERF_TIMER_END );
	fperf_TimerReport( FPERF_SWAP, FPERF_TIMER_START );

	if(Gameloop_bSwapEnabled)
	{
		if( !fvid_Swap() ) {
			return FALSE;
		}
	}

	fperf_TimerReport( FPERF_SWAP, FPERF_TIMER_END );
	fperf_TimerReport( FPERF_DRAW, FPERF_TIMER_START );

	// Do game drawing...
	PROTRACK_BEGINBLOCK("Draw");

	fvid_Begin();

	if(Gameloop_bDrawEnabled)
	{
		if( _pFcnDraw ) {
			if( !_pFcnDraw() ) {
				PROTRACK_ENDBLOCK();	//"Draw
				fvid_End();
				return FALSE;
			}
		}
	}

	PROTRACK_DRAW();

	#if !FANG_PRODUCTION_BUILD
		if( _bDisplayScreenSafeArea ) {
			_DrawScreenSafeArea();	
		}
	#endif
	
		fvid_End();

	fperf_TimerReport( FPERF_DRAW, FPERF_TIMER_END );

	PROTRACK_ENDBLOCK();	//"Draw

	fperf_ModuleUpdate();
 
	return TRUE;
}

#if FANG_PLATFORM_WIN
static BOOL _IsEscKeyHit( void ) {
	// under windows, look for the escape key
#if defined(MA_PC_INPUT)
	// Escape releases mouse look and pauses; the window close button/Alt-F4 exits.
	return FALSE;
#else
	return (GetAsyncKeyState( VK_ESCAPE ) < 0);
#endif
}

static BOOL _ShouldAppMinimize( void ) {
	// under windows, look for the backspace key
#if defined(MA_PC_INPUT)
	return pcinput_KeyHeld( VK_BACK );
#else
	return (GetAsyncKeyState( VK_BACK ) < 0);
#endif
}
#endif

static BOOL _InitGameSystems( void ) {
	s32 i;

	// Init game systems...
	for( i=0; _aSystemFunctionArray[i].pFcnInitSystem; i++ ) {
		if( !_aSystemFunctionArray[i].pFcnInitSystem() ) {
#if !FANG_PRODUCTION_BUILD		
			DEVPRINTF( "_InitGameSystems() - Init() failed for MA game system %s.\n", _aSystemFunctionArray[i].pszSysName );
#endif
			// Error initializing a game system.
			// Uninit all systems we've already initialized in reverse order...
			for( i--; i >= 0; i-- ) {
				_aSystemFunctionArray[i].pFcnUninitSystem();
			}

			// Return back error...
			return FALSE;
		}
	}


	// All game systems initialized successfully...
	return TRUE;
}


// This routine loads all systems which require that the language already be specified
// and the Game phrase table already be loaded with language specific phrases
BOOL gameloop_InitLocalizedGameSystems( void ) {
	s32 i;

	// Init game systems...
	for( i=0; _aLocalizedSystemFunctionArray[i].pFcnInitSystem; i++ ) {
		if( !_aLocalizedSystemFunctionArray[i].pFcnInitSystem() ) {
#if !FANG_PRODUCTION_BUILD		
			DEVPRINTF( "gameloop_InitLocalizedGameSystems() - Init() failed for localized MA game system %s.\n", _aLocalizedSystemFunctionArray[i].pszSysName );
#endif			
			// Error initializing a game system.
			// Uninit all systems we've already initialized in reverse order...
			for( i--; i >= 0; i-- ) {
				_aLocalizedSystemFunctionArray[i].pFcnUninitSystem();
			}

			// Return back error...
			return FALSE;
		}
	}

	// All localized game systems initialized successfully...
	return TRUE;
}


static void _UninitGameSystems( void ) {
	s32 i;

	// Count the number of entries in _aLocalizedSystemFunctionArray[]...
	for( i=0; _aLocalizedSystemFunctionArray[i].pFcnInitSystem; i++ ) {}

	// Uninit game systems...
	for( i--; i>=0; i-- ) {
		_aLocalizedSystemFunctionArray[i].pFcnUninitSystem();
	}
	
	// Count the number of entries in _aSystemFunctionArray[]...
	for( i=0; _aSystemFunctionArray[i].pFcnInitSystem; i++ ) {}

	// Uninit game systems...
	for( i--; i>=0; i-- ) {
		_aSystemFunctionArray[i].pFcnUninitSystem();
	}
}


#if !FANG_PRODUCTION_BUILD
	static void _SetupScreenSafeAreaVtxs() {
		_pViewportOrtho3D = fviewport_Create();
		fviewport_InitOrtho3D( _pViewportOrtho3D, 0.1f, 100.0f );

		_aScreenSafeArea[0].Pos_MS.Set( -_SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.x,
										-_SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.y,
										1.0f );
		_aScreenSafeArea[0].ColorRGBA.Set( _SCREEN_SAFE_AREA_R, _SCREEN_SAFE_AREA_G, _SCREEN_SAFE_AREA_B, _SCREEN_SAFE_AREA_A );
		_aScreenSafeArea[0].ST.Set( 0.0f, 0.0f );

		_aScreenSafeArea[1].Pos_MS.Set( -_SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.x,
										_SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.y,
										1.0f );
		_aScreenSafeArea[1].ColorRGBA.Set( _SCREEN_SAFE_AREA_R, _SCREEN_SAFE_AREA_G, _SCREEN_SAFE_AREA_B, _SCREEN_SAFE_AREA_A );
		_aScreenSafeArea[1].ST.Set( 0.0f, 0.0f );

		_aScreenSafeArea[2].Pos_MS.Set( _SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.x,
										_SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.y,
										1.0f );
		_aScreenSafeArea[2].ColorRGBA.Set( _SCREEN_SAFE_AREA_R, _SCREEN_SAFE_AREA_G, _SCREEN_SAFE_AREA_B, _SCREEN_SAFE_AREA_A );
		_aScreenSafeArea[2].ST.Set( 0.0f, 0.0f );

		_aScreenSafeArea[3].Pos_MS.Set( _SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.x,
										-_SCREEN_AREA_PERCENT_USABLE * _pViewportOrtho3D->HalfRes.y,
										1.0f );
		_aScreenSafeArea[3].ColorRGBA.Set( _SCREEN_SAFE_AREA_R, _SCREEN_SAFE_AREA_G, _SCREEN_SAFE_AREA_B, _SCREEN_SAFE_AREA_A );
		_aScreenSafeArea[3].ST.Set( 0.0f, 0.0f );

		_aScreenSafeArea[4] = _aScreenSafeArea[0];
	}

	static void _DrawScreenSafeArea() {
		CFXfm CamXfm;

		FViewport_t *pPrevViewport = fviewport_GetActive();
		frenderer_Push( FRENDERER_DRAW, NULL );
		fviewport_SetActive( _pViewportOrtho3D );
		
		CamXfm.Identity();
		CamXfm.InitStackWithView();
	
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_SetTexture( NULL );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, _aScreenSafeArea, _NUM_SCREEN_SAFE_VTXS );

#if _DRAW_CONTROLS
		CFVec3 Center, Pt, Pt2;
		CFVec2 Stick;
		CFColorRGBA DialRGBA;

		DialRGBA.Set( _CONTROL_INTENSITY, _CONTROL_INTENSITY, _CONTROL_INTENSITY, _CONTROL_ALPHA );
		
		// draw the left stick frame
		Center.Set( 0.48f * _pViewportOrtho3D->HalfRes.x, 
					0.15f * _pViewportOrtho3D->HalfRes.y,
					1.0f );		
		fdraw_FacetedCircle( &CFVec3A::m_UnitAxisZ.v3, &Center, _CONTROL_RADIUS, &DialRGBA );
		// draw the left stick cross hair
		Pt = Center;
		Pt.x -= _CONTROL_RADIUS;
		Pt2 = Center;
		Pt2.x += _CONTROL_RADIUS;
		fdraw_SolidLine( &Pt, &Pt2, &DialRGBA );
		Pt = Center;
		Pt.y -= _CONTROL_RADIUS;
		Pt2 = Center;
		Pt2.y += _CONTROL_RADIUS;
		fdraw_SolidLine( &Pt, &Pt2, &DialRGBA );
		// draw the left stick direction and mag
		Stick.Set( Gamepad_aapSample[0][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState,
				   Gamepad_aapSample[0][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState );
		Stick *= _CONTROL_RADIUS;
		Pt = Stick;
		Pt += Center;
		fdraw_SolidLine( &Center, &Pt );

		ftext_DebugPrintf( 0.67f, 0.39f, "X = %.3f\nY = %.3f\nMag = %.1f\nAng = %.0f",
							Gamepad_aapSample[0][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState,
							Gamepad_aapSample[0][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState,
							Stick.Mag(),
							FMATH_RAD2DEG( fmath_Atan( Gamepad_aapSample[0][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState, Gamepad_aapSample[0][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState ) ) );
		
		// draw the right stick frame
		Center.Set( 0.73f * _pViewportOrtho3D->HalfRes.x, 
					0.15f * _pViewportOrtho3D->HalfRes.y,
					1.0f );	
		fdraw_FacetedCircle( &CFVec3A::m_UnitAxisZ.v3, &Center, _CONTROL_RADIUS, &DialRGBA );
		// draw the right stick cross hair
		Pt = Center;
		Pt.x -= _CONTROL_RADIUS;
		Pt2 = Center;
		Pt2.x += _CONTROL_RADIUS;
		fdraw_SolidLine( &Pt, &Pt2, &DialRGBA );
		Pt = Center;
		Pt.y -= _CONTROL_RADIUS;
		Pt2 = Center;
		Pt2.y += _CONTROL_RADIUS;
		fdraw_SolidLine( &Pt, &Pt2, &DialRGBA );
		// draw the right stick direction and mag
		Stick.Set( Gamepad_aapSample[0][GAMEPAD_MAIN_ROTATE_LEFT_RIGHT]->fCurrentState, 
				   Gamepad_aapSample[0][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState );
		Stick *= _CONTROL_RADIUS;
		Pt = Stick;
		Pt += Center;
		fdraw_SolidLine( &Center, &Pt );

		ftext_DebugPrintf( 0.81f, 0.39f, "x = %.3f\ny = %.3f\nMag = %.1f\nAng = %.0f",
							Gamepad_aapSample[0][GAMEPAD_MAIN_ROTATE_LEFT_RIGHT]->fCurrentState,
							Gamepad_aapSample[0][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState,
							Stick.Mag(),
							FMATH_RAD2DEG( fmath_Atan( Gamepad_aapSample[0][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState, Gamepad_aapSample[0][GAMEPAD_MAIN_ROTATE_LEFT_RIGHT]->fCurrentState ) ) );
#endif
		
		frenderer_Pop();
		fviewport_SetActive( pPrevViewport );
	}
#endif


void gameloop_DisablePlatformReset( void ) {
	_bPlatformResetDisabled = TRUE;
}

BOOL gameloop_CheckForPlatformReset( BOOL bExitImmediately ) {
	BOOL bNeedToReset = FALSE;
	
#if FANG_PLATFORM_GC
	if( !_bPlatformResetDisabled ) {
		if( _bResetButtonDown ) {
			if( !OSGetResetButtonState() ) {
				if( !bExitImmediately ) {
					gameloop_ScheduleExit();
					bNeedToReset = TRUE;
				} else {
					// Do the reset RIGHT NOW.
					fvid_DestroyWindow();
					OSResetSystem( OS_RESET_RESTART, 0, FALSE );
				}
			}
		} else {
			_bResetButtonDown = OSGetResetButtonState();
		}
	}
#endif
	return bNeedToReset;
}


