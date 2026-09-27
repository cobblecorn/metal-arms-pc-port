//////////////////////////////////////////////////////////////////////////////////////
// Hud2.cpp - (New) HUD Class for Mettle Arms.
//
// Author: Justin Link
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
// 02/05/02 Link		Created.
// 03/11/02 Link		Integrated into new shell.
// 02/xx/03 Stanfill	Cleaned up to make non-static and usable for multiplayer
// 04/19/03 sranck		Officially deemed this module as a disaster and condemned it.
//////////////////////////////////////////////////////////////////////////////////////

#include "Hud2.h"
#include "fresload.h"
#include "frenderer.h"
#include "fdraw.h"
#include "floop.h"
#include "ftext.h"
#include "fclib.h"
#include "fworld_coll.h"
#include "Item.h"
#include "ItemInst.h"
#include "gamepad.h"
#include "FScalarObj.h"
#include "player.h"
#include "meshtypes.h"
#include "reticle.h"
#include "fvtxpool.h"
#include "bot.h"
#include "game.h"
#include "itemRepository.h"
#include "collectable.h"
#include "ai/aigameutils.h"
#include "MultiplayerMgr.h"
#include "vehicle.h"
#include "weapon.h"
#include "pausescreen.h"
#include "meshentity.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#include "wpr_drawutils.h"
#endif

#if FANG_PLATFORM_GC
#include <wchar.h> //for _vsnwprintf
#endif

#define _RADAR_ON		TRUE


#define HUD2_AMMO_ALERT_FREQ				2.0f

#define _TRANSMISSION_ANTENNA_MESH_NAME		"GRDGradio01"
#define _TRANSMISSION_ANTENNA_ATTACH_BONE	"AntennaE"
#define _TRANSMISSION_ANTENNA_SCALE			1.0f
#define _TRANSMISSION_ANTENNA_OFFSET_Y		0.41f
#define _TRANSMISSION_MAX_AUTHOR_TEXT_LEN	40
#define _TRANSMISSION_FADEOUT_SECS			0.5f
#define _TRANSMISSION_FLASH_SECS			0.5f
#define _TRANSMISSION_BOX_ALPHA				0.8f

#define _RADAR_BLIP_COLOR_FRIENDLY_R	1.0f
#define _RADAR_BLIP_COLOR_FRIENDLY_G	1.0f
#define _RADAR_BLIP_COLOR_FRIENDLY_B	0.1f

#define _RADAR_BLIP_COLOR_ENEMY_R		1.0f
#define _RADAR_BLIP_COLOR_ENEMY_G		0.1f
#define _RADAR_BLIP_COLOR_ENEMY_B		0.1f

#define _TEXTDISPLAY_FLASH_SECS			0.5f
#define _TEXTDISPLAY_FADEOUT_TIME		0.5f
#define _TEXTDISPLAY_OO_FADEOUT_TIME	( 1.0f / _TEXTDISPLAY_FADEOUT_TIME )



// =============================================================================================================

// An 'R' postfix means that it has delay + repeat behavior.
enum
{
	JINPUT_UP			= 1 << 0,
	JINPUT_DOWN			= 1 << 1,
	JINPUT_LEFT			= 1 << 2,
	JINPUT_RIGHT		= 1 << 3,
	JINPUT_LBUTTONR		= 1 << 4,
	JINPUT_RBUTTONR		= 1 << 5,
	JINPUT_WSLEFT		= 1 << 6,
	JINPUT_WSRIGHT		= 1 << 7,
	JINPUT_LANALOGUPR	= 1 << 8,
	JINPUT_LANALOGDOWNR	= 1 << 9,
	JINPUT_LANALOGLEFT	= 1 << 10,
	JINPUT_LANALOGRIGHT	= 1 << 11,
	JINPUT_RIGHTBUTTON	= 1 << 12,
	JINPUT_UPR			= 1 << 13,		// DPad up with repeat
	JINPUT_DOWNR		= 1 << 14,

//	JINPUT_MOVEUP = (JINPUT_UP | JINPUT_RBUTTON | JINPUT_LANALOGUP),
//	JINPUT_MOVEDOWN = (JINPUT_DOWN | JINPUT_LBUTTON | JINPUT_LANALOGDOWN),
	JINPUT_MOVEUP = (JINPUT_UPR | JINPUT_RBUTTONR | JINPUT_LANALOGUPR),
	JINPUT_MOVEDOWN = (JINPUT_DOWNR | JINPUT_LBUTTONR | JINPUT_LANALOGDOWNR),

	JINPUT_NONE = 0
};

// =============================================================================================================

FINLINE f32 P2JX(u32 uPixel) { return(((f32)(uPixel) - 320.0f) * (1.0f / 320.0f)); }
FINLINE f32 P2JY(u32 uPixel) { return(((f32)(uPixel) - 240.0f) * (-1.0f / 240.0f) * 0.75f); }
FINLINE f32 P2AX(u32 uPixel) { return((f32)(uPixel) * (1.0f / 640.0f)); }
FINLINE f32 P2AY(u32 uPixel) { return((f32)(uPixel) * (1.0f / 480.0f) * 0.75f); }

// Utility functions for convert from my coordinates to ftext coordinates.
f32 JCXToFTextX(f32 fX);
f32 JCYToFTextY(f32 fY);
f32 FTextXToJCX(f32 fX);
f32 FTextYToJCX(f32 fY);

// Local versions of ftext_Print... functions that transform everything correctly first
static void _XFormTextArea( const FTextAreaHandle_t ohArea, f32* afSavedCoords );
static void _RestoreTextArea( const FTextAreaHandle_t ohArea, f32* afSavedCoords );
static void _XFormPrintfLocal( f32 fX, f32 fY, cchar *pszFormat, ... );

static BOOL _bPauseToSwitchWeapons = TRUE; // SET TO TRUE TO PAUSE THE ACTION WHILE SWITCHING WEAPONS, FALSE TO KEEP THE ACTION GOING, BUT DON'T ALLOW THE SWITCHING PLAYER TO MOVE WHILE THEY ARE SWITCHING

// =============================================================================================================

// =============================================================================================================

// Specified constants (feel free to tweak).
static const f32 fBatteryMeterBorderX = 0.01f * 2.0f;			// 1% off the left edge.
static const f32 fBatteryMeterBorderY = 0.01f * 1.5f;			// 1% off the top edge.
static const f32 fBatteryMeterSize = 0.085f;						// Size of one battery vertically, scaling is proportional.
static const f32 fBatteryMeterSpacing = fBatteryMeterSize;
static const f32 fHealthBarOfsY = 0.007f * 1.5f;					// Y offset of health bar (relative to battery).
static const f32 fHealthBarHeight = 0.025f;
static const f32 fItemPickupYawOmega = 8.0f;//5.0f;
static const f32 fItemPickupScrollOffTime = 0.1f;
static const f32 fItemPickupBounceFreq = 8.0f;
static const f32 fItemPickupBounceAmplitude = 0.3f;
static const f32 fHealthBarParticleDensity = 40.0f;				// Number of particles per 1.0 of health.
static const f32 fItemPickupJumpTime = 2.0f;//2.0f;

static const f32 _fEnergyOffsetX = 0.011f;		// Offset of energy bar relative to battery.

static const f32 fWeaponIconWidth = P2JX(589) - P2JX(525);//0.215f;//0.225f;//0.15f * 2.0f;
static const f32 fWeaponIconHeight = P2JY(363) - P2JY(427);//0.215f;//0.225f;//0.15f * 1.5f;

// DFS -- Old
//static const f32 afWeaponLE[2] = { P2JX(591) - fWeaponIconWidth, P2JX(640 - 591) };//{ (1.0f - 0.08f * 2.0f) - fWeaponIconWidth, (-1.0f + 0.08f * 2.0f) + 0.0f };
//static const f32 afWeaponTE[2] = { P2JY(427) + fWeaponIconHeight, P2JY(427) + fWeaponIconHeight };//{ (-0.75f + 0.08f * 1.5f) + fWeaponIconHeight, (-0.75f + 0.08f * 1.5f) + fWeaponIconHeight };
// DFS -- New
static const f32 afWeaponLE[2] = { P2JX(635) - fWeaponIconWidth, P2JX(5) };//{ (1.0f - 0.08f * 2.0f) - fWeaponIconWidth, (-1.0f + 0.08f * 2.0f) + 0.0f };
static const f32 afWeaponTE[2] = { P2JY(464) + fWeaponIconHeight, P2JY(464) + fWeaponIconHeight };//{ (-0.75f + 0.08f * 1.5f) + fWeaponIconHeight, (-0.75f + 0.08f * 1.5f) + fWeaponIconHeight };

static const f32 fWSSpacing = 0.05f * 0.75f;//0.01f * 0.75f;//0.1f * 0.75f;

// Box describing the weapon information for single player paused select
static CFVec3 fWSInfoBoxUL(P2JX(143), P2JY(83), 0.0f);
static CFVec3 fWSInfoBoxLR(P2JX(519), P2JY(462), 0.0f);
static CFVec2 CHud2_vecInfoBoxULST(0.0f, 0.0f);
static CFVec2 CHud2_vecInfoBoxLRST(0.734f, 0.740f/*0.740f, 0.740f*/);

static const f32 CHud2_fFadeOutAlpha = 0.5f;
static const f32 CHud2_fDelayConstant = 2.8f;//2.8f;//1.8f;//2.0f;//2.4f;		// Don't ask me to quantify what this really means.

static const f32 afWeaponLEOffScreen[2] = { 1.0f, -1.0f - fWeaponIconWidth };
static const u32 uMaxWSItemsVisible = 6;
static const f32 fWSScrollingOnTime = 0.15f;
static const f32 CHud2_fDelay = 2.0f;
static const f32 fWSScrollingToTime = 0.1f;
static const u32 auActivateButton[2] = { JINPUT_WSRIGHT, JINPUT_WSLEFT };
static const CFColorRGBA argbaArrowFlash[2] = { CFColorRGBA(0.25f, 0.25f, 1.0f, 1.0f), CFColorRGBA(0.8f, 0.8f, 0.8f, 1.0f) };
static const f32 fWSInfoBoxOffset = 0.20f;										// This is how to offset the above box.
static const f32 afWSInfoBoxTextLE[2] = { 0.1f, 0.45f };
static const f32 afWSInfoBoxTextRE[2] = { 0.55f, 0.9f };
static const f32 fWSInfoBoxTextTE = ((0.0f * -1.5f) + 0.75f) * 0.5f;			// In ftext coordinates.
static const f32 fWSInfoBoxTextBE = ((-0.24f * -1.5f) + 0.75f) * 0.5f;			// In ftext coordinates.

static const f32 fWSInfoBoxMeshLE = -0.24f * 2.0f;
static const f32 fWSInfoBoxMeshRE = 0.24f * 2.0f;
static const f32 fWSInfoBoxMeshTE = 0.29f * 1.5f;
static const f32 fWSInfoBoxMeshBE = -0.01f * 1.5f;

static const f32 fWSItemOmegaYaw = 2.0f;
static const f32 fWSAmmoTextWidth = 0.10f;										// In ftext coordinates.
static const f32 fWSAmmoTextHeight = 0.025f;									// In ftext coordinates.
static const f32 fWSFlashDuration = 0.05f;//0.1f;
static const f32 fBPMovementTime = 3.0f;

// This assumes a square radar display with the player at the center.
static const f32 fRadarWidth = 0.32f;//0.40f;
static const f32 fRadarHeight = 0.32f;//0.20f;
static const f32 fRadarRange = 200.0f;
static const f32 fRadarFadeOutTime = 1.0f;//5.0f;
static const f32 fRadarPosX = 0.98f - fRadarWidth; //P2JX(485);//0.5f;
static const f32 fRadarPosY = 0.735f; //P2JY(40);//-0.2f;

static const f32 fQSMinTime = 0.5f;
static const f32 fQSMaxTime = 3.5f;
static const f32 fQSArrowLE = 0.56f;
static const f32 fQSArrowTE = -0.44f;
static const f32 fQSArrowWidth = 0.1f;
static const f32 fQSArrowHeight = 0.1f;
static const f32 fQSArrowFlashFreq = 20.0f;
static const f32 fWasherIconLE = 0.3f;			// Washer's x position, our coords (right is 1.0)
static const f32 fWasherIconTE = 0.748f;			// washer picture's y position, our coords (top is 0.75)
static const f32 fWasherTextY = 0.02f;			// Washer text y position in viewport, text coords (top is 0)
static const f32 fWasherWidth = 0.2f;
static const f32 fWasherHeight = 0.2f;
static const f32 fWasherSlideTime = 0.4f;
static const f32 fQSFlashDuration = 1.0f;
static const u32 CHud2_uNumFlashes = 3;
static const f32 CHud2_fWSInfoBoxAlpha = 0.4f;

static CFVec2 CHud2_avecWSNameUL[2] = { CFVec2(P2AX(155), P2AY(250+35)), CFVec2(P2AX(640 - 297), P2AY(250+35)) };
static CFVec2 CHud2_avecWSNameLR[2] = { CFVec2(P2AX(302), P2AY(280+35)), CFVec2(P2AX(640 - 150), P2AY(280+35)) };

static CFVec2 CHud2_avecWSDescUL[2] = { CFVec2(P2AX(165), P2AY(285+35)), CFVec2(P2AX(640 - 440), P2AY(285+35)) };
static CFVec2 CHud2_avecWSDescLR[2] = { CFVec2(P2AX(440), P2AY(360+35)), CFVec2(P2AX(640 - 155), P2AY(360+35)) }; // y used to be 337

static CFVec2 CHud2_avecWSEUKUL[2] = { CFVec2(P2AX(305), P2AY(358+35)), CFVec2(P2AX(640 - 450-2), P2AY(358+35)) };
static CFVec2 CHud2_avecWSEUKLR[2] = { CFVec2(P2AX(460), P2AY(371+35)), CFVec2(P2AX(640 - 307+10), P2AY(371+35)) };

static CFVec2 CHud2_avecWSAmmoAlertUL[2] = { CFVec2(P2AX(300), P2AY(210+35)), CFVec2(P2AX(640 - 450), P2AY(210+35)) };
static CFVec2 CHud2_avecWSAmmoAlertLR[2] = { CFVec2(P2AX(450), P2AY(225+35)), CFVec2(P2AX(640 - 307), P2AY(225+35)) };

static CFVec2 CHud2_avecWSUpgradeableUL[2] = { CFVec2(P2AX(300-25), P2AY(210-7)), CFVec2(P2AX(640 - 450-25), P2AY(210-7)) };
static CFVec2 CHud2_avecWSUpgradeableLR[2] = { CFVec2(P2AX(450+25), P2AY(240-7)), CFVec2(P2AX(640 - 307+25), P2AY(240-7)) };

static CFVec2 CHud2_vecWSNoScopeUL = CFVec2(P2AX(640 - 450), P2AY(190+40));
static CFVec2 CHud2_vecWSNoScopeLR = CFVec2(P2AX(640 - 307), P2AY(230+40));

static CFVec2 CHud2_avecWSWeapTypeUL[2] = { CFVec2(P2AX(219), P2AY(384+35)), CFVec2(P2AX(640 - 410), P2AY(384+35)) };
static CFVec2 CHud2_avecWSWeapTypeLR[2] = { CFVec2(P2AX(395), P2AY(422+35)), CFVec2(P2AX(640 - 219), P2AY(422+35)) };

static const f32 CHud2_fWasherOmegaY = 7.0f;

static const f32 CHudBatterySystem_fEnergyChargeTime = 4.0f;
static const f32 CHudBatterySystem_fEnergyChargeRate = 1.0f;	// Energy units/sec.

static const f32 CHudBatterySystem_fBPOmegaY = 8.0f;

static const f32 CHudBatterySystem_fBPPauseDur = 2.0f;
static const f32 CHudBatterySystem_fBPStabilizeDur = 2.0f;
static const f32 CHudBatterySystem_fBPFlyDur = 1.0f;

static const f32 CHudBatterySystem_fBPIconWidth = 0.1f;
static const f32 CHudBatterySystem_fBPIconHeight = 0.2f;

static CFVec3 CHudBatterySystem_avecBPCurvePoints[6] = {	CFVec3(0.0f/*-0.0015f*/, 0.0f/*-0.1270f*/, 0.0000f),
																CFVec3(+0.1253f + 0.0f, 0.3414f + 0.1f, 0.0000f),
																CFVec3(+0.5535f + 0.15f, 0.6371f + 0.2f, 0.0000f),
																CFVec3(+0.8510f + 0.1f, 0.4661f + 0.1f, 0.0000f), 
																CFVec3(+0.5612f + 0.1f + 0.2f, 0.2965f - 0.2f, 0.0000f),
																CFVec3(-0.5137f, 0.6355f, 0.0000f) };
static const f32 CHudBatterySystem_afBPCoeff[6] = { 1.0f, 5.0f, 10.0f, 10.0f, 5.0f, 1.0f };

// DFS -- Old
//static CFVec2 CHud2_avecAmmoBoxUL[2] = { CFVec2(P2JX(417), P2JY(396)), CFVec2(P2JX(105), P2JY(396)) };
//static CFVec3 CHud2_avecAmmoBoxLR[2] = { CFVec2(P2JX(555), P2JY(438)), CFVec2(P2JX(192), P2JY(438)) };
// DFS -- New
static CFVec2 CHud2_avecAmmoBoxUL[2] = { CFVec2(P2JX(461), P2JY(433)), CFVec2(P2JX(55-12), P2JY(433)) };
static CFVec3 CHud2_avecAmmoBoxLR[2] = { CFVec2(P2JX(599), P2JY(475)), CFVec2(P2JX(142), P2JY(475)) };

// First Idx is Prim/Sec, second is Reg/Mil.
static CFVec2 CHud2_avecAmmoBoxST1[2][2] = { { CFVec2(0.000f, 0.750f), CFVec2(/*0.270f*/0.278f, 0.750f) }, { CFVec2(0.000f, 0.837f), CFVec2(0.320f, 0.837f) } };	//CFVec2(0.277f, 0.837f) } };
static CFVec2 CHud2_avecAmmoBoxST2[2][2] = { { CFVec2(0.270f, 0.833f), CFVec2(/*0.488f*/0.495f, 0.833f) }, { CFVec2(0.269f, 0.920f), CFVec2(0.451171875f, 0.920f) } };	//CFVec2(0.492f, 0.920f) } };

// DFS -- Old
//static CFVec2 CHud2_avecCurAmmoUL[2] = { CFVec2(P2AX(443), P2AY(405)), CFVec2(P2AX(125), P2AY(405)) };
//static CFVec2 CHud2_avecCurAmmoLR[2] = { CFVec2(P2AX(493), P2AY(429)), CFVec2(P2AX(168), P2AY(429)) };
//static CFVec2 CHud2_avecMaxAmmoUL[2] = { CFVec2(P2AX(507), P2AY(422)), CFVec2(P2AX(96), P2AY(422)) };
//static CFVec2 CHud2_avecMaxAmmoLR[2] = { CFVec2(P2AX(540), P2AY(434)), CFVec2(P2AX(122), P2AY(434)) };
// DFS -- New
static CFVec2 CHud2_avecCurAmmoUL[2] = { CFVec2(P2AX(487), P2AY(442)), CFVec2(P2AX(81-15), P2AY(442)) };
static CFVec2 CHud2_avecCurAmmoLR[2] = { CFVec2(P2AX(537), P2AY(466)), CFVec2(P2AX(124-15), P2AY(466)) };
static CFVec2 CHud2_avecMaxAmmoUL[2] = { CFVec2(P2AX(551), P2AY(459)), CFVec2(P2AX(52), P2AY(459)) };
static CFVec2 CHud2_avecMaxAmmoLR[2] = { CFVec2(P2AX(584), P2AY(471)), CFVec2(P2AX(78), P2AY(471)) };
static const char CHud2_acAmmoFont[2] = { '6', '4'}; // used to be 5

static const f32 CHud2_fWSBoxTime = 0.15f;//4.0f;

static const f32 CHud2_fSDCounterHalfWidth = 0.04f;
static const f32 CHud2_fSDCounterHalfDelWidth = 0.03f;
static const f32 CHud2_fSDCounterTime = 1.0f;

#if 0// what the fuck Justin, this is stupid...you have this in multiple places, I'm using the one in the pause screen - Mike Starich
static const char *CHud2_pszControlsTex = "tfh2bttnsxb";
	#if FANG_PLATFORM_DX
		static CFVec2 CHud2_avecButtonST1[5] = { CFVec2( 0.0f / 255.0f, 27.0f / 127.0f), CFVec2(69.0f / 255.0f, 27.0f / 127.0f), CFVec2(102.0f / 255.0f, 27.0f / 127.0f), CFVec2(134.0f / 255.0f, 0.20f), CFVec2(162.0f / 255.0f, 0.20f) };
		static CFVec2 CHud2_avecButtonST2[5] = { CFVec2(31.0f / 255.0f, 59.0f / 127.0f), CFVec2(100.0f / 255.0f, 59.0f / 127.0f), CFVec2(133.0f / 255.0f, 59.0f / 127.0f), CFVec2(161.0f / 255.0f, 0.47f), CFVec2(189.0f / 255.0f, 0.47f)  };
	#elif FANG_PLATFORM_GC
		static CFVec2 CHud2_avecButtonST1[5] = { CFVec2( 0.0f / 255.0f, 27.0f / 127.0f), CFVec2(33.0f / 255.0f, 27.0f / 127.0f), CFVec2(102.0f / 255.0f, 27.0f / 127.0f), CFVec2(134.0f / 255.0f, 0.20f), CFVec2(162.0f / 255.0f, 0.20f) };
		static CFVec2 CHud2_avecButtonST2[5] = { CFVec2(31.0f / 255.0f, 59.0f / 127.0f), CFVec2(64.0f / 255.0f, 59.0f / 127.0f), CFVec2(133.0f / 255.0f, 59.0f / 127.0f), CFVec2(161.0f / 255.0f, 0.47f), CFVec2(189.0f / 255.0f, 0.47f)  };
	#elif FANG_PLATFORM_PS2				//CPS 4.7.03
		static CFVec2 CHud2_avecButtonST1[5] = { CFVec2( 0.0f / 255.0f, 27.0f / 127.0f), CFVec2(33.0f / 255.0f, 27.0f / 127.0f), CFVec2(102.0f / 255.0f, 27.0f / 127.0f), CFVec2(134.0f / 255.0f, 0.20f), CFVec2(162.0f / 255.0f, 0.20f) };		//CPS 4.7.03
		static CFVec2 CHud2_avecButtonST2[5] = { CFVec2(31.0f / 255.0f, 59.0f / 127.0f), CFVec2(64.0f / 255.0f, 59.0f / 127.0f), CFVec2(133.0f / 255.0f, 59.0f / 127.0f), CFVec2(161.0f / 255.0f, 0.47f), CFVec2(189.0f / 255.0f, 0.47f)  };	//CPS 4.7.03
	#else
		#error <Include platform-specific ...>
	#endif
#endif


// Derived constants (do not change these directly!)
static const f32 fBatteryMeterLE = -1.0f + fBatteryMeterBorderX;
static const f32 fBatteryMeterTE = 0.75f - fBatteryMeterBorderY;
static const f32 fOOItemPickupJumpTime = 1.0f / fItemPickupJumpTime;
static const f32 fOOWSScrollingOnTime = 1.0f / fWSScrollingOnTime;
static const f32 fOOWSScrollingToTime = 1.0f / fWSScrollingToTime;
static const f32 fOOWSFlashDuration = 1.0f / fWSFlashDuration;
static const f32 fOOBPMovementTime = 1.0f / fBPMovementTime;

static const f32 fOORadarRange = 1.0f / fRadarRange;
static const f32 fOORadarFadeOutTime = 1.0f / fRadarFadeOutTime;

static const f32 fOOWasherSlideTime = 1.0f / fWasherSlideTime;
static const f32 CHudBatterySystem_fOOEnergyChargeTime = 1.0f / CHudBatterySystem_fEnergyChargeTime;
static const f32 CHudBatterySystem_fHalfBPIconWidth = 0.5f * CHudBatterySystem_fBPIconWidth;
static const f32 CHudBatterySystem_fHalfBPIconHeight = 0.5f * CHudBatterySystem_fBPIconHeight;
static const f32 CHudBatterySystem_fOOBPFlyDur = 1.0f / CHudBatterySystem_fBPFlyDur;
static const f32 CHud2_fOOWSDelayedScrollingOnTime = 1.0f / (fWSScrollingOnTime - CHud2_fDelay);
static const f32 CHud2_fOOWSBoxTime = 1.0f / CHud2_fWSBoxTime;
static const f32 CHud2_fOOSDCounterTime = 1.0f / CHud2_fSDCounterTime;

// Detonation pack icon. Lets us know the time remaining on a detonation pack.
// Positioned relative to the battery now.
static const f32 fDetPackPosX1   = fBatteryMeterLE; // FTextXToJCX(52.0f / 640.0f);
static const f32 fDetPackPosY1   = fBatteryMeterTE - (1.0f * fBatteryMeterSize); // FTextYToJCX(66.0f / 480.0f);
static const f32 fDetPackWidth   = 0.175f;
static const f32 fDetPackHeight  = 0.26875f;
static const f32 fDetPackPosX2   = fDetPackPosX1 + fDetPackWidth; //FTextXToJCX(108.0f / 640.0f);
static const f32 fDetPackPosY2   = fDetPackPosY1 - fDetPackHeight; //FTextYToJCX(152.0f / 480.0f);

// race timer icon.
static const f32 fRaceTimerPosX1   = fBatteryMeterLE;
static const f32 fRaceTimerPosY1   = fBatteryMeterTE - (1.0f * fBatteryMeterSize);
static const f32 fRaceTimerWidth   = 0.175f;
static const f32 fRaceTimerHeight  = 0.26875f;
static const f32 fRaceTimerPosX2   = fRaceTimerPosX1 + fRaceTimerWidth;
static const f32 fRaceTimerPosY2   = fRaceTimerPosY1 - fRaceTimerHeight;

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

static const u32 uDynamicParticlePoolSize = 1;//100;
static const u32 uHealthBarParticlePoolSize = 1;//50;
//static const u32 uClusterParticlePoolSize = 0;//250;//1000;
static const f32 fClusterFadeInTime = 3.0f;
static const f32 fClusterFadeOutTime = 3.0f;

static const f32 fOOClusterFadeInTime = 1.0f / fClusterFadeInTime;
static const f32 fOOClusterFadeOutTime = 1.0f / fClusterFadeOutTime;

// RAF OLD
//static const f32 fAmmoBoxHeight = fWeaponIconHeight * 0.452f;
//static CFVec2 CHudWSItem_avecFrameOfs[2] = { CFVec2(fWeaponIconHeight * (-27.0f / 64.0f) - 0.015f, -fWeaponIconHeight * 0.55f), CFVec2(fWeaponIconHeight * (37.0f / 64.0f) + fAmmoBoxHeight * (1.904f) + 0.015f, -fWeaponIconHeight * 0.55f) };
//static CFVec2 CHudWSItem_avecFrameRect[2] = { CFVec2(fAmmoBoxHeight * (1.904f), fAmmoBoxHeight), CFVec2(-fAmmoBoxHeight * (1.904f), fAmmoBoxHeight) };
// RAF NEW
static const f32 fAmmoBoxHeight = fWeaponIconHeight * 0.527f; //.502 before messing around
static CFVec2 CHudWSItem_avecFrameOfs[2] = { CFVec2(fWeaponIconHeight * (-37.0f / 64.0f) - 0.015f, -fWeaponIconHeight * 0.475f), CFVec2(fWeaponIconHeight * (37.0f / 64.0f) + fAmmoBoxHeight * (1.904f) + 0.015f, -fWeaponIconHeight * 0.475f) };
static CFVec2 CHudWSItem_avecFrameRect[2] = { CFVec2(fAmmoBoxHeight * (2.0f), fAmmoBoxHeight), CFVec2(-fAmmoBoxHeight * (2.0f), fAmmoBoxHeight) };

static CFVec2 CHudWSItem_avecEUKRect[2] = { CFVec2(0.08f, 0.08f), CFVec2(0.08f, 0.08f) };
static CFVec2 CHudWSItem_avecEUKOfs[2] = { CFVec2(-0.09f, 0.0f), CFVec2(fWeaponIconWidth, 0.0f) };
//RAF OLD
//static CFVec2 CHudWSItem_avecEUKST1[4] = { CFVec2(0.000f, 0.000f), CFVec2(0.750f, 0.394f), CFVec2(0.902f, 0.394f), CFVec2(0.750f, 0.250f) };
//static CFVec2 CHudWSItem_avecEUKST2[4] = { CFVec2(0.000f, 0.000f), CFVec2(0.847f, 0.500f), CFVec2(1.000f, 0.500f), CFVec2(0.844f, 0.351f) };
//RAF NEW
static CFVec2 CHudWSItem_avecEUKST1[4] = { CFVec2(0.000f, 0.000f), CFVec2(0.000f, 0.394f), CFVec2(0.152f, 0.394f), CFVec2(0.000f, 0.250f) };
static CFVec2 CHudWSItem_avecEUKST2[4] = { CFVec2(0.000f, 0.000f), CFVec2(0.097f, 0.500f), CFVec2(0.250f, 0.500f), CFVec2(0.094f, 0.351f) };

// First index is primary/secondary, second index is cur/max.
// RAF OLD
//static CFVec2 CHudWSItem_avecAmmoTextOfs[2][2] = { { CFVec2(-0.045f, 0.060f), CFVec2(-0.045f, 0.077f) }, { CFVec2(0.5f * fWeaponIconWidth + 0.01f, 0.060f), CFVec2(0.5f * fWeaponIconWidth + 0.01f, 0.077f) } };
//static CFVec2 CHudWSItem_vecAmmoTextRect = CFVec2(0.04f, 0.017f);
// RAF NEW
static CFVec2 CHudWSItem_avecAmmoTextOfs[2][2] = { { CFVec2(-0.056f, 0.053f), CFVec2(-0.056f, 0.075f) }, { CFVec2(0.5f * fWeaponIconWidth + 0.012f, 0.053f), CFVec2(0.5f * fWeaponIconWidth + 0.012f, 0.075f) } };
static CFVec2 CHudWSItem_vecAmmoTextRect = CFVec2(0.05f, 0.017f);

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

CFTexInst CHudWSItem::m_tex;

// =============================================================================================================

CHudWSItem::CHudWSItem()
{
}

// =============================================================================================================

CHudWSItem::~CHudWSItem()
{
}

// =============================================================================================================

void CHudWSItem::Init()
{
/*	if(m_tex.GetTexDef() == NULL)
	{
		m_tex.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2hudall1")) );
		FASSERT(m_tex.GetTexDef() != NULL);
	}*/

	// Note : We leave some of these fields uninitialized (specifically, the position).
	// You initialize those with an explicit call to UpdateTextArea().
	FTextArea_t oTextArea;

	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.ohFont = '4';
	oTextArea.oColorBackground.Blue();
	oTextArea.oColorBackground.fAlpha = 0.5f;
//	oTextArea.fBorderThicknessX = 0.002f;
//	oTextArea.fBorderThicknessY = 0.002f;
	oTextArea.oColorBorder.OpaqueBlue();
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_RIGHT;
	oTextArea.bVisible = FALSE;

//	m_hAmmoText = ftext_Create(&oTextArea);
	m_ahAmmoText[0] = ftext_Create(&oTextArea);
	m_ahAmmoText[1] = ftext_Create(&oTextArea);
}

// =============================================================================================================

void CHudWSItem::UpdateTextArea(u32 uSide)
{
	f32 fX1, fX2, fY1, fY2;

	FASSERT(uSide >= 0);
	FASSERT(uSide <= 1);

	u32 uTemp;
	for(uTemp = 0; uTemp < 2; ++uTemp)
	{
		fX1 = JCXToFTextX(m_vecCurUL.x) + CHudWSItem_avecAmmoTextOfs[uSide][uTemp].x;
		fX2 = fX1 + CHudWSItem_vecAmmoTextRect.x;
		fY1 = JCYToFTextY(m_vecCurUL.y) + CHudWSItem_avecAmmoTextOfs[uSide][uTemp].y;
		fY2 = fY1 + CHudWSItem_vecAmmoTextRect.y;

		ftext_GetAttributes(m_ahAmmoText[uTemp])->fUpperLeftX = fX1;
		ftext_GetAttributes(m_ahAmmoText[uTemp])->fUpperLeftY = fY1;
		ftext_GetAttributes(m_ahAmmoText[uTemp])->fLowerRightX = fX2;
		ftext_GetAttributes(m_ahAmmoText[uTemp])->fLowerRightY = fY2;
	}
}

// =============================================================================================================

BOOL CHudWSItem::ShouldWeDisplayScopeUnusableInfo( CHudWSItem *pWSItem, CHudWSItem *pOppositeWSItem ) {
	if( pWSItem && pWSItem->m_pItemInst && pWSItem->m_pItemInst->m_pWeapon && (pWSItem->m_pItemInst->m_pWeapon->Type() == CWeapon::WEAPON_TYPE_SCOPE) && (CHud2::GetAmmoAlertUnitCountdownTimer() < 0.75f) ) {
		if( pOppositeWSItem ) {
			if( pOppositeWSItem->m_pItemInst ) {
				if( pOppositeWSItem->m_pItemInst->m_pWeapon ) {
					if( pOppositeWSItem->m_pItemInst->m_pWeapon->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_SCOPE_ENABLED ) {
						return FALSE;
					}
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}

// The responsibility of this method is to draw the WS item's icon, the ammo box, the EUK upgrade
//   meter, and the text.
//void CHudWSItem::Draw(BOOL bIsSelected, u32 uSide, BOOL bDrawHighlight)
void CHudWSItem::Draw(BOOL bIsSelected, u32 uSide, BOOL bDrawHighlight, BOOL bOverride/*=FALSE*/, CItem *pOverrideItem/*=NULL*/, BOOL bReverse/*=FALSE*/, CHudWSItem *pOppositeItem/*=NULL*/  )
{
	if( !m_pItemInst ) {
		return;
	}

	// We will need our viewport dimensions
	FViewport_t* pVP = fviewport_GetActive();

	CWeapon *pWeapon = m_pItemInst->m_pWeapon;

	// Get our current upper left in viewport space
	CFVec3 vTopLeft;
	FXfm_pModel->TransformPointF(vTopLeft, m_vecCurUL);

	// Don't draw the icon if it is above the top of the screen.
	if(vTopLeft.y > pVP->HalfRes.y)
		return;

	FDrawVtx_t avtxList[16];
	//// Draw the icon.
	//
	u32 uCurVtx = 0;
	f32 fAlpha = 1.0f;

#if 1
	if( pWeapon ) {
		CWeapon::AmmoAlert_e nAmmoAlert = pWeapon->ComputeAmmoAlert();

		if( nAmmoAlert == CWeapon::AMMO_ALERT_NO_AMMO ) {
			fAlpha = 0.4f;
		}
	}

//	f32 fAlpha = bIsSelected ? 1.0f : 0.25f;
#endif

	CItem *pItem;
	if( bOverride ) {
		pItem = pOverrideItem;
	} else {
		if ( !m_pItemInst || !m_pItemInst->m_pItemData )
		{
			return;
		}
		pItem = m_pItemInst->m_pItemData;
	}

	f32 fS1 = pItem->m_vecST1.x;
	f32 fT1 = pItem->m_vecST1.y;
	f32 fS2 = pItem->m_vecST2.x;
	f32 fT2 = pItem->m_vecST2.y - 0.005f;

	if( bReverse ) {
		FMATH_FSWAP( fS1, fS2 );
	}

	f32 fScaledWidth, fScaledHeight;
	CFVec3 ScaledCurUL;

	fScaledWidth = fWeaponIconWidth;
	fScaledHeight = fWeaponIconHeight;
	ScaledCurUL = m_vecCurUL;

	if( bDrawHighlight ) {
		f32 fAmp = 0.0125f * fmath_Sin( CHud2::GetAmmoAlertUnitCountdownTimer() * FMATH_2PI );

		fScaledWidth += fAmp;
		fScaledHeight += fAmp;

		ScaledCurUL.x -= 0.5f * fAmp;
		ScaledCurUL.y += 0.5f * fAmp;
	}

	avtxList[uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS = ScaledCurUL;
	avtxList[uCurVtx].ST.Set(fS1, fT1);
	uCurVtx++;

	avtxList[uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS = ScaledCurUL;
	avtxList[uCurVtx].Pos_MS.x += fScaledWidth;
	avtxList[uCurVtx].ST.Set(fS2, fT1);
	uCurVtx++;

	avtxList[uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS = ScaledCurUL;
	avtxList[uCurVtx].Pos_MS.x += fScaledWidth;
	avtxList[uCurVtx].Pos_MS.y -= fScaledHeight;
	avtxList[uCurVtx].ST.Set(fS2, fT2);
	uCurVtx++;

	avtxList[uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS = ScaledCurUL;
	avtxList[uCurVtx].Pos_MS.y -= fScaledHeight;
	avtxList[uCurVtx].ST.Set(fS1, fT2);
	uCurVtx++;

	fdraw_SetTexture(&pItem->m_texIcon);
	fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, avtxList, uCurVtx);

	/////
	// NOTE THAT THE REMAINING DRAWS IN THIS FUNCTION USE THE SAME TEXTURE, SO THEY ARE ACCUMULATED
	/////
	uCurVtx = 0;

	if(bDrawHighlight)
	{
		f32 fAlpha = 1.0f;

		f32 fS1 = 0.255f;
		f32 fT1 = 0.25f;
		f32 fS2 = 0.50f;
		f32 fT2 = 0.50f;

		if(uSide == 1)
		{
			FMATH_FSWAP( fS1, fS2 );
		}

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].ST.Set(fS1, fT1);
		uCurVtx++;

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].Pos_MS.x += fScaledWidth;
		avtxList[uCurVtx].ST.Set(fS2, fT1);
		uCurVtx++;

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].Pos_MS.x += fScaledWidth;
		avtxList[uCurVtx].Pos_MS.y -= fScaledHeight;
		avtxList[uCurVtx].ST.Set(fS2, fT2);
		uCurVtx++;

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].Pos_MS.y -= fScaledHeight;
		avtxList[uCurVtx].ST.Set(fS1, fT2);
		uCurVtx++;
	}

	if( (CHud2::GetAmmoAlertUnitCountdownTimer() < 0.75f) && ShouldWeDisplayScopeUnusableInfo( this, pOppositeItem ) ) {
		f32 fAlpha = 1.0f;

		f32 fS1 = 0.255f;
		f32 fT1 = 0.50f;
		f32 fS2 = 0.50f;
		f32 fT2 = 0.75f;

		if(uSide == 1)
		{
			FMATH_FSWAP( fS1, fS2 );
		}

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].ST.Set(fS1, fT1);
		uCurVtx++;

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].Pos_MS.x += fScaledWidth;
		avtxList[uCurVtx].ST.Set(fS2, fT1);
		uCurVtx++;

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].Pos_MS.x += fScaledWidth;
		avtxList[uCurVtx].Pos_MS.y -= fScaledHeight;
		avtxList[uCurVtx].ST.Set(fS2, fT2);
		uCurVtx++;

		avtxList[uCurVtx].ColorRGBA.White();
		avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
		avtxList[uCurVtx].Pos_MS = ScaledCurUL;
		avtxList[uCurVtx].Pos_MS.y -= fScaledHeight;
		avtxList[uCurVtx].ST.Set(fS1, fT2);
		uCurVtx++;
	}
	//
	//// Draw the icon.

	if( bOverride ) 
	{
		// Draw any verts in the list
		if ( uCurVtx )
		{
			fdraw_SetTexture( &m_tex );
			fdraw_PrimList( FDRAW_PRIMTYPE_QUADLIST, avtxList, uCurVtx );
		}
		// we're done, just get out
		return;
	}

	// Fudge the top edge of the current weapon up a bit to protect against
	// dirty floating point numbers
	f32 fTopEdge = afWeaponTE[uSide] + 0.35f * fWeaponIconHeight;

	CHud2* pHud = CHud2::GetCurrentHud();

	if( (m_pItemInst->m_aeDisplayType[0] != CItemInst::AMMODISPLAY_NONE) || (m_pItemInst->m_aeDisplayType[1] != CItemInst::AMMODISPLAY_NONE) )
	{
		if((pHud->m_eCurHudMode != HUDMODE_MIL) && (m_vecCurUL.y > fTopEdge))
		{
			//// Draw the ammo frame.
			//

			// RAF OLD
			//fS1 = 0.5f;
			//fS2 = 0.5f + 0.238f;
			//fT1 = 0.25f;
			//fT2 = 0.25f + 0.125f;

			// RAF NEW
			//fS1 = 0.5f;
			//fS2 = 0.5f + 0.250f; //moved box 5 pixels over
			//fT1 = 0.266f; //moved box 4 pixels down
			//fT2 = 0.266f + 0.141f; //added 4 pixels to length

			// RAF NEWEST
			fS1 = 0.5f;
			fS2 = 0.5f + 0.28f; //moved box 15 pixels over
			fT1 = 0.266f; //moved box 4 pixels down
			fT2 = 0.266f + 0.141f; //added 4 pixels to length

			f32	fX1 = m_vecCurUL.x + CHudWSItem_avecFrameOfs[uSide].x;
			f32 fX2 = fX1 + CHudWSItem_avecFrameRect[uSide].x;
			f32 fY1 = m_vecCurUL.y + CHudWSItem_avecFrameOfs[uSide].y;
			f32 fY2 = fY1 - CHudWSItem_avecFrameRect[uSide].y;

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
			avtxList[uCurVtx].ST.Set(fS1, fT1);
			uCurVtx++;

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
			avtxList[uCurVtx].ST.Set(fS2, fT1);
			uCurVtx++;

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
			avtxList[uCurVtx].ST.Set(fS2, fT2);
			uCurVtx++;

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
			avtxList[uCurVtx].ST.Set(fS1, fT2);
			uCurVtx++;

			//
			//// Draw the ammo frame.
		}
	}

	if( (m_vecCurUL.y > fTopEdge))
	{
		//// Write the ammo text.
		//
		m_pItemInst->PrintItemText(m_ahAmmoText[0], m_ahAmmoText[1], NULL, NULL, FALSE, FALSE);
		//
		//// Write the ammo text.
	}

	if(pHud->m_eCurHudMode != HUDMODE_MIL)
	{
		if (m_vecCurUL.y > (afWeaponTE[uSide] - 0.2f * fWeaponIconHeight))
		{
			//// Draw the EUK upgrade.
			//
			u32 uCurEUK = m_pItemInst->m_pItemData->m_uCurLevel;

			fS1 = (uSide == 0) ? CHudWSItem_avecEUKST1[uCurEUK].x : CHudWSItem_avecEUKST2[uCurEUK].x;
			fS2 = (uSide == 0) ? CHudWSItem_avecEUKST2[uCurEUK].x : CHudWSItem_avecEUKST1[uCurEUK].x;
			fT1 = CHudWSItem_avecEUKST1[uCurEUK].y;
			fT2 = CHudWSItem_avecEUKST2[uCurEUK].y;

			f32 fX1 = m_vecCurUL.x + CHudWSItem_avecEUKOfs[uSide].x;
			f32 fX2 = fX1 + CHudWSItem_avecEUKRect[uSide].x;
			f32 fY1 = m_vecCurUL.y + CHudWSItem_avecEUKOfs[uSide].y;
			f32 fY2 = fY1 - CHudWSItem_avecEUKRect[uSide].y;

			if(uSide == 1)
			{
				fX1 += 0.009f;
				fX2 += 0.009f;
			}

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
			avtxList[uCurVtx].ST.Set(fS1, fT1);
			uCurVtx++;

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
			avtxList[uCurVtx].ST.Set(fS2, fT1);
			uCurVtx++;

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
			avtxList[uCurVtx].ST.Set(fS2, fT2);
			uCurVtx++;

			avtxList[uCurVtx].ColorRGBA.White();
			avtxList[uCurVtx].ColorRGBA.fAlpha = 1.0f;
			avtxList[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
			avtxList[uCurVtx].ST.Set(fS1, fT2);
			uCurVtx++;

			//
			//// Draw the EUK upgrade.
		}
	}

	if ( uCurVtx )
	{
		fdraw_SetTexture( &m_tex );
		fdraw_PrimList( FDRAW_PRIMTYPE_QUADLIST, avtxList, uCurVtx );
	}
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================
CHudRadarEntity::CHudRadarEntity()
{
	m_bIsActive = FALSE;
	m_pWM = NULL;
	m_fStationaryTime = 0.0f;
}

// =============================================================================================================

CHudRadarEntity::~CHudRadarEntity()
{
}


// =============================================================================================================

//void CHudRadarEntity::CalcBSPos(CFXfm *pXfmBlinkToWorld)
void CHudRadarEntity::CalcBSPos(CBot *pPlayer)
{
	m_vecPos_BS = m_vecPos_WS;
	m_vecPos_BS.Sub(pPlayer->MtxToWorld()->m_vPos);
	// Set it to inactive if it's off the radar.
	if(m_vecPos_BS.MagSq() >= fRadarRange * fRadarRange)
	{
		m_bIsActive = FALSE;
		m_fStationaryTime = fRadarFadeOutTime;
	}
	m_vecPos_BS = pPlayer->WS2MS(m_vecPos_BS);
	m_vecPos_BS.Mul(fOORadarRange * 0.47f * fRadarHeight);
	m_vecPos_BS.x += (fRadarPosX + 0.5f * fRadarWidth);
	m_vecPos_BS.z += (fRadarPosY - 0.5f * fRadarHeight);
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

// =============================================================================================================

CHudRadarBlip::CHudRadarBlip()
{
	m_bIsActive = FALSE;
}

// =============================================================================================================

CHudRadarBlip::~CHudRadarBlip()
{
}

// ============================================================================
void CHudRadarBlip::Set(CFVec3 *vecPos, CFColorRGB *oColor, f32 fLifetime, f32 fSpeed, f32 fInitSize)
{
	m_bIsActive = TRUE;
	m_fLifetime = fLifetime;
	m_fOOLifetime = 1.0f / fLifetime;
	m_fSpeed = fSpeed;
	m_fTimer = 0.0f;
	m_rgbColor = *oColor;
	m_vecPos = *vecPos;
	m_fInitSize = fInitSize;
}

// ============================================================================
void CHudRadarBlip::Work()
{
	if(m_bIsActive)
	{
		m_fTimer += FLoop_fPreviousLoopSecs;
		if(m_fTimer >= m_fLifetime)
			m_bIsActive = FALSE;
	}
}

// =============================================================================================================

void CHudRadarBlip::AddRBToVtxList(FDrawVtx_t *avtxList, u32 &uCurVtx)
{
	f32 fAlpha = 1.0f - 0.99f * m_fTimer * m_fOOLifetime;
	FASSERT_UNIT_FLOAT(fAlpha);

	f32 fDist = m_fInitSize + m_fTimer * m_fSpeed;
	f32 fX1 = m_vecPos.x - fDist;
	f32 fX2 = m_vecPos.x + fDist;
	f32 fY1 = m_vecPos.y + fDist;
	f32 fY2 = m_vecPos.y - fDist;

	avtxList[uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = m_rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	++uCurVtx;
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

#if 0
enum ParticleClusterState_e
{
	PCSTATE_IDLE,
	PCSTATE_APPEARING,
	PCSTATE_FLOATING,
	PCSTATE_DISAPPEARING
};

class CHudParticle
{
public:
	CHudParticle();
	void Clear();
	~CHudParticle();

	static void InitSystem();

	static void SetParticleTextures(FTexDef_t *ptexParticle1, FTexDef_t *ptexParticle2);
	static void SetHudMode(HudMode_e eNewHudMode);

	static void SetParticleSoupRegion(CFVec2 *vecUL, CFVec2 *vecLR);

	static void CheckSoup();

	static void AllocParticlePatch(CFVec2 *vecUL, CFVec2 *vecLR, f32 fSize, f32 fMeanLifeTime, f32 fLifeTimeDist, u32 uNumParticles);
	static void ResetParticleSoup(CFVec2 *vecUL, CFVec2 *vecLR, f32 fNewSize, u32 uNumParticles);
	static void AllocParticleSoup(f32 fSize/*, CFColorRGBA rgbaColor1, CFColorRGBA rgbaColor2*/, u32 uNumParticles);
//	static void SpawnParticleCluster(CFVec3 *vecCenter, f32 fRadiusX, f32 fRadiusY, f32 fSize, f32 fMeanLifeTime, f32 fLifeTimeDist, u32 uNumParticles);
//	static void SetClusterPoint(CFVec3 *vecCenter, f32 fStrength);
//	static void KillParticleClusters();
	static void Work();
	static void Draw(f32 fXSoupOffset, CFColorRGB rgbSoupColor);

	static void Shutdown();

private:
	CFVec2 m_vecPos, m_vecVel;
	f32 m_fSize;
//	union
//	{
	f32 m_fAge, m_fLifeTime;
	f32 m_fPulsateOfs;
//	};
	BOOL m_bInUse;

	static CFVec2 m_vecSoupUL, m_vecSoupLR;
	static u32 m_uNumSoupParticles;

	static BOOL m_bSoupSystemInitialized;

	// Related to the current hud mode.
	static CFTexInst m_atexParticle[2];
	static HudMode_e m_eCurHudMode;
	static CFColorRGB m_rgbWashColor;

	static CFTexInst *m_ptexCur;

	static f32 m_fStrength;			// of particle cluster springs.
	static ParticleClusterState_e m_ePCState;
	static CFVec3 m_vecClusterPoint;
	static f32 m_fPCCounter, m_fPulsateTimer;

	FCLASS_STACKMEM_NOALIGN(CHudParticle);
};

// =============================================================================================================

BOOL CHudParticle::m_bSoupSystemInitialized = FALSE;
CHudParticle CHudParticle_aoPool[uDynamicParticlePoolSize];
CHudParticle CHudParticle_aoSoupPool[uHealthBarParticlePoolSize];
//CHudParticle CHudParticle_aoClusterPool[uClusterParticlePoolSize];
CFTexInst *CHudParticle::m_ptexCur;
CFVec2 CHudParticle::m_vecSoupUL, CHudParticle::m_vecSoupLR;
u32 CHudParticle::m_uNumSoupParticles = 0;
f32 CHudParticle::m_fStrength;
ParticleClusterState_e CHudParticle::m_ePCState;
CFVec3 CHudParticle::m_vecClusterPoint;
f32 CHudParticle::m_fPCCounter, CHudParticle::m_fPulsateTimer;

CFTexInst CHudParticle::m_atexParticle[2];
HudMode_e CHudParticle::m_eCurHudMode;
CFColorRGB CHudParticle::m_rgbWashColor;

// =============================================================================================================

CHudParticle::CHudParticle()
{
	m_bInUse = FALSE;
}

// =============================================================================================================

CHudParticle::~CHudParticle()
{
	m_bInUse = FALSE;
	m_fAge = -1.0f;
}

// =============================================================================================================

void CHudParticle::InitSystem()
{
	m_ePCState = PCSTATE_IDLE;
	m_fPCCounter = 0.0f;
}

// =============================================================================================================

void CHudParticle::SetHudMode(HudMode_e eNewHudMode)
{
	m_eCurHudMode = eNewHudMode;
	switch(eNewHudMode)
	{
		case HUDMODE_GLITCH:
		case HUDMODE_SLOSH:
		case HUDMODE_KRUNK:
		case HUDMODE_MOZER:
		{
			m_ptexCur = &m_atexParticle[0];
			m_rgbWashColor.Set(0.0f, 0.0f, 1.0f);
			break;
		}
		case HUDMODE_MIL:
		{
			m_ptexCur = &m_atexParticle[1];
			m_rgbWashColor.Set(0.709f, 0.0f, 0.0f);
			break;
		}
	}
}

// =============================================================================================================

void CHudParticle::SetParticleTextures(FTexDef_t *ptexParticle1, FTexDef_t *ptexParticle2)
{
	m_atexParticle[0].SetTexDef( ptexParticle1 );
	m_atexParticle[1].SetTexDef( ptexParticle2 );
}

// =============================================================================================================

void CHudParticle::SetParticleSoupRegion(CFVec2 *vecUL, CFVec2 *vecLR)
{
	m_vecSoupUL = *vecUL;
	m_vecSoupLR = *vecLR;
	m_bSoupSystemInitialized = TRUE;
}

// =============================================================================================================

void CHudParticle::CheckSoup()
{
	u32 uPartIdx;
	CHudParticle *pCurPart;

	for(uPartIdx = 0; uPartIdx < m_uNumSoupParticles; ++uPartIdx)
	{
		pCurPart = &(CHudParticle_aoSoupPool[uPartIdx]);

		if(pCurPart->m_bInUse)
		{
			f32 fSize = pCurPart->m_fSize;
		}
	}
}

// =============================================================================================================

void CHudParticle::AllocParticlePatch(CFVec2 *vecUL, CFVec2 *vecLR, f32 fSize, f32 fMeanLifeTime, f32 fLifeTimeDist, u32 uNumParticles)
{
	u32 uNewPartNum, uNewPartIdx;
	uNewPartIdx = 0;
	for(uNewPartNum = 0; uNewPartNum < uNumParticles; ++uNewPartNum)
	{
		FASSERT(uNewPartIdx <= uDynamicParticlePoolSize);
		while((uNewPartIdx < uDynamicParticlePoolSize) && (CHudParticle_aoPool[uNewPartIdx].m_bInUse))
		{
			++uNewPartIdx;
		}

		if(uNewPartIdx == uDynamicParticlePoolSize)
			break;
		FASSERT(uNewPartIdx <= uDynamicParticlePoolSize);

		CHudParticle_aoPool[uNewPartIdx].m_bInUse = TRUE;
		CHudParticle_aoPool[uNewPartIdx].m_fAge = 0.0f;
		CHudParticle_aoPool[uNewPartIdx].m_fSize = fSize;
		CHudParticle_aoPool[uNewPartIdx].m_fLifeTime = fMeanLifeTime + fmath_RandomBipolarUnitFloat() * fLifeTimeDist;

		f32 fWeight1, fWeight2;

		fWeight1 = fmath_RandomFloat();
		fWeight2 = 1.0f - fWeight1;
		CHudParticle_aoPool[uNewPartIdx].m_vecPos.x = (fWeight1 * vecUL->x) + (fWeight2 * vecLR->x);

		fWeight1 = fmath_RandomFloat();
		fWeight2 = 1.0f - fWeight1;
		CHudParticle_aoPool[uNewPartIdx].m_vecPos.y = (fWeight1 * vecUL->y) + (fWeight2 * vecLR->y);

		f32 fTheta = fmath_RandomFloatRange(0.0f, FMATH_2PI);
		fmath_SinCos(fTheta, &(CHudParticle_aoPool[uNewPartIdx].m_vecVel.y), &(CHudParticle_aoPool[uNewPartIdx].m_vecVel.x));
		CHudParticle_aoPool[uNewPartIdx].m_vecVel *= (fmath_RandomFloat() * 0.5f/* * 0.25f*/);
		CHudParticle_aoPool[uNewPartIdx].m_vecVel.y += 0.1f;
		++uNewPartIdx;
	}
}

// =============================================================================================================

void CHudParticle::AllocParticleSoup(f32 fSize/*, CFColorRGBA rgbaColor1, CFColorRGBA rgbaColor2*/, u32 uNumParticles)
{
	u32 uLastToBeAllocated;
	CHudParticle *pCurPart;

	FMATH_CLAMPMAX(uNumParticles, (uHealthBarParticlePoolSize - m_uNumSoupParticles));
	uLastToBeAllocated = m_uNumSoupParticles + uNumParticles;

	for(; m_uNumSoupParticles < uLastToBeAllocated; ++m_uNumSoupParticles)
	{
		pCurPart = &(CHudParticle_aoSoupPool[m_uNumSoupParticles]);

		f32 fWeight1, fWeight2;

		fWeight1 = fmath_RandomFloat();
		fWeight2 = 1.0f - fWeight1;
		pCurPart->m_vecPos.x = fWeight1 * (m_vecSoupUL.x + fSize) + fWeight2 * (m_vecSoupLR.x - fSize);

		fWeight1 = fmath_RandomFloat();
		fWeight2 = 1.0f - fWeight1;
		pCurPart->m_vecPos.y = fWeight1 * (m_vecSoupUL.y - fSize) + fWeight2 * (m_vecSoupLR.y + fSize);

		f32 fTheta = fmath_RandomFloatRange(0.0f, FMATH_2PI);
		fmath_SinCos(fTheta, &(pCurPart->m_vecVel.y), &(pCurPart->m_vecVel.x));
		pCurPart->m_vecVel *= fmath_RandomFloatRange(0.0f, 0.08f);

		pCurPart->m_fSize = fSize;
		pCurPart->m_bInUse = TRUE;
	}
}

// =============================================================================================================

void CHudParticle::ResetParticleSoup(CFVec2 *vecUL, CFVec2 *vecLR, f32 fNewSize, u32 uNumParticles)
{
	u32 uTotalParticleCount = 0;
	u32 uCurPartIdx;
	CHudParticle *pCurPart;

	m_vecSoupLR = *vecLR;
	m_vecSoupUL = *vecUL;

	//// Remove any offending particles.
	//
	for(uCurPartIdx = 0; uCurPartIdx < uHealthBarParticlePoolSize; ++uCurPartIdx)
	{
		pCurPart = &(CHudParticle_aoSoupPool[uCurPartIdx]);
		if(pCurPart->m_bInUse)
		{
			f32 fSize = pCurPart->m_fSize;
			if(pCurPart->m_vecPos.x - fSize < vecUL->x)
			{
//				DEVPRINTF("Killing %d.\n", uCurPartIdx);
				pCurPart->m_bInUse = FALSE;
				continue;
			}
			if(pCurPart->m_vecPos.x + fSize > vecLR->x)
			{
//				DEVPRINTF("Killing %d.\n", uCurPartIdx);
				pCurPart->m_bInUse = FALSE;
				continue;
			}
			if(pCurPart->m_vecPos.y + fSize > vecUL->y)
			{
//				DEVPRINTF("Killing %d.\n", uCurPartIdx);
				pCurPart->m_bInUse = FALSE;
				continue;
			}
			if(pCurPart->m_vecPos.y - fSize < vecLR->y)
			{
//				DEVPRINTF("Killing %d.\n", uCurPartIdx);
				pCurPart->m_bInUse = FALSE;
				continue;
			}
//			DEVPRINTF("%d survived.\n", uCurPartIdx);
			++uTotalParticleCount;
		}
	}
	//
	////

	if(uTotalParticleCount > uNumParticles)
	{
		while(uTotalParticleCount > uNumParticles)
		{
			--uTotalParticleCount;
		}
	}
	else if(uTotalParticleCount < uNumParticles)
	{
		if((m_vecSoupLR.x - m_vecSoupUL.x) <= (2.0f * fNewSize))
		{
			// No particles are going to fit in here anyway.
			return;
		}

		uCurPartIdx = 0;
		while(uTotalParticleCount < uNumParticles)
		{
			while(CHudParticle_aoPool[uCurPartIdx].m_bInUse)
				++uCurPartIdx;
			if(uCurPartIdx >= uHealthBarParticlePoolSize)
				break;
			FASSERT(uCurPartIdx < uHealthBarParticlePoolSize);

			pCurPart = &(CHudParticle_aoSoupPool[uCurPartIdx]);
//			DEVPRINTF("Reusing %d.\n", uCurPartIdx);

			f32 fWeight1, fWeight2;

			pCurPart->m_bInUse = TRUE;

			fWeight1 = fmath_RandomFloat();
			fWeight2 = 1.0f - fWeight1;
			pCurPart->m_vecPos.x = fWeight1 * (m_vecSoupUL.x + fNewSize) + fWeight2 * (m_vecSoupLR.x - fNewSize);

			fWeight1 = fmath_RandomFloat();
			fWeight2 = 1.0f - fWeight1;
			pCurPart->m_vecPos.y = fWeight1 * (m_vecSoupUL.y - fNewSize) + fWeight2 * (m_vecSoupLR.y + fNewSize);

			f32 fTheta = fmath_RandomFloatRange(0.0f, FMATH_2PI);
			fmath_SinCos(fTheta, &(pCurPart->m_vecVel.y), &(pCurPart->m_vecVel.x));
			pCurPart->m_vecVel *= fmath_RandomFloatRange(0.0f, 0.08f);

			pCurPart->m_fSize = fNewSize;
			++uCurPartIdx;
			++uTotalParticleCount;
		}
	}
}

// =============================================================================================================
#if 0
void CHudParticle::SpawnParticleCluster(CFVec3 *vecCenter, f32 fRadiusX, f32 fRadiusY, f32 fSize, f32 fMeanLifeTime, f32 fLifeTimeDist, u32 uNumParticles)
{
	u32 uNewPartNum, uNewPartIdx;
	uNewPartIdx = 0;
	for(uNewPartNum = 0; uNewPartNum < uNumParticles; ++uNewPartNum)
	{
		FASSERT(uNewPartIdx < uClusterParticlePoolSize);
		while(CHudParticle_aoClusterPool[uNewPartIdx].m_bInUse)
		{
			++uNewPartIdx;
			if(uNewPartIdx == uClusterParticlePoolSize)
				break;
		}

		if(uNewPartIdx == uClusterParticlePoolSize)
			break;
		FASSERT(uNewPartIdx < uClusterParticlePoolSize);

		CHudParticle_aoClusterPool[uNewPartIdx].m_bInUse = TRUE;
		CHudParticle_aoClusterPool[uNewPartIdx].m_fSize = fSize;
		f32 fWeight1, fWeight2;

		fWeight1 = fmath_RandomFloat();
		fWeight2 = 1.0f - fWeight1;
		CHudParticle_aoClusterPool[uNewPartIdx].m_vecPos.x = fRadiusX * fmath_RandomBipolarUnitFloat();
		CHudParticle_aoClusterPool[uNewPartIdx].m_vecPos.x += vecCenter->x;

		fWeight1 = fmath_RandomFloat();
		fWeight2 = 1.0f - fWeight1;
		CHudParticle_aoClusterPool[uNewPartIdx].m_vecPos.y = fRadiusY * fmath_RandomBipolarUnitFloat();
		CHudParticle_aoClusterPool[uNewPartIdx].m_vecPos.y += vecCenter->y;

		f32 fTheta = fmath_RandomFloatRange(0.0f, FMATH_2PI);
		fmath_SinCos(fTheta, &(CHudParticle_aoClusterPool[uNewPartIdx].m_vecVel.y), &(CHudParticle_aoClusterPool[uNewPartIdx].m_vecVel.x));
		CHudParticle_aoClusterPool[uNewPartIdx].m_vecVel *= (fmath_RandomFloat() * 0.2f/* * 0.25f*/);
		CHudParticle_aoClusterPool[uNewPartIdx].m_vecVel.y += 0.1f;

		CHudParticle_aoClusterPool[uNewPartIdx].m_fPulsateOfs = fmath_RandomFloat() * FMATH_2PI;
		++uNewPartIdx;
	}

	m_ePCState = PCSTATE_APPEARING;
	m_fPCCounter = 0.0f;
}
#endif
// =============================================================================================================
#if 0
void CHudParticle::SetClusterPoint(CFVec3 *vecCenter, f32 fStrength)
{
	m_vecClusterPoint = *vecCenter;
	m_fStrength = fStrength;
}
#endif
// =============================================================================================================
#if 0
void CHudParticle::KillParticleClusters()
{
	m_ePCState = PCSTATE_DISAPPEARING;
}
#endif
// =============================================================================================================

void CHudParticle::Work()
{
	u32 uPartIdx;
	CHudParticle *pCurPart;
#if 0
	//// Do state transitions for cluster particles.
	//
	switch(m_ePCState)
	{
		case PCSTATE_IDLE:
		{
			break;
		}
		case PCSTATE_APPEARING:
		{
			m_fPCCounter += FLoop_fPreviousLoopSecs;
			if(m_fPCCounter >= fClusterFadeInTime)
			{
				m_fPCCounter = 0.0f;
				m_ePCState = PCSTATE_FLOATING;
			}

			break;
		}
		case PCSTATE_DISAPPEARING:
		{
			m_fPCCounter += FLoop_fPreviousLoopSecs;
			if(m_fPCCounter >= fClusterFadeOutTime)
			{
				u32 uCurPCIdx;
				for(uCurPCIdx = 0; uCurPCIdx < uClusterParticlePoolSize; ++uCurPCIdx)
				{
					CHudParticle_aoClusterPool[uCurPCIdx].m_bInUse = FALSE;
				}

				m_fPCCounter = 0.0f;
				m_ePCState = PCSTATE_IDLE;
			}

			break;
		}
		case PCSTATE_FLOATING:
		{
			// The transition out of this state is triggered externally.
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
	//
	////
#endif
	//// Do work for dynamic particles.
	//
	for(uPartIdx = 0; uPartIdx < uDynamicParticlePoolSize; ++uPartIdx)
	{
		pCurPart = &(CHudParticle_aoPool[uPartIdx]);
		if(pCurPart->m_bInUse)
		{
			pCurPart->m_fAge += FLoop_fPreviousLoopSecs;
			if(pCurPart->m_fAge >= pCurPart->m_fLifeTime)
			{
				pCurPart->m_bInUse = FALSE;
				continue;
			}

			pCurPart->m_vecVel.y -= 0.4f * FLoop_fPreviousLoopSecs;

			pCurPart->m_vecPos += pCurPart->m_vecVel * FLoop_fPreviousLoopSecs;
		}
	}
	//
	////

	//// Do work for soup particles.
	//
	FASSERT(m_uNumSoupParticles <= uHealthBarParticlePoolSize);
	for(uPartIdx = 0; uPartIdx < uHealthBarParticlePoolSize; ++uPartIdx)
	{
		pCurPart = &(CHudParticle_aoSoupPool[uPartIdx]);
		if(pCurPart->m_bInUse)
		{
			pCurPart->m_vecPos += pCurPart->m_vecVel * FLoop_fPreviousLoopSecs;

			f32 fSize = pCurPart->m_fSize;
			if(pCurPart->m_vecPos.x - fSize < m_vecSoupUL.x)
			{
				pCurPart->m_vecPos.x = m_vecSoupUL.x + fSize;
//				FASSERT(pCurPart->m_vecVel.x < 0.0f);
				pCurPart->m_vecVel.x *= -1.0f;
			}

			if(pCurPart->m_vecPos.x + fSize > m_vecSoupLR.x)
			{
				pCurPart->m_vecPos.x = m_vecSoupLR.x - fSize;
//				FASSERT(pCurPart->m_vecVel.x > 0.0f);
				pCurPart->m_vecVel.x *= -1.0f;
			}

			if(pCurPart->m_vecPos.y - fSize < m_vecSoupLR.y)
			{
				pCurPart->m_vecPos.y = m_vecSoupLR.y + fSize;
//				FASSERT(pCurPart->m_vecVel.y < 0.0f);
				pCurPart->m_vecVel.y *= -1.0f;
			}

			if(pCurPart->m_vecPos.y + fSize > m_vecSoupUL.y)
			{
				pCurPart->m_vecPos.y = m_vecSoupUL.y - fSize;
//				FASSERT(pCurPart->m_vecVel.y > 0.0f);
				pCurPart->m_vecVel.y *= -1.0f;
			}
		}
	}
	//
	////
#if 0
	////
	//
	for(uPartIdx = 0; uPartIdx < uClusterParticlePoolSize; ++uPartIdx)
	{
		pCurPart = &(CHudParticle_aoClusterPool[uPartIdx]);
		if(pCurPart->m_bInUse)
		{
			pCurPart->m_fAge += FLoop_fPreviousLoopSecs;

			CFVec3 vecForce = m_vecClusterPoint;
			vecForce -= pCurPart->m_vecPos;
			vecForce *= m_fStrength;

			pCurPart->m_vecVel += vecForce * FLoop_fPreviousLoopSecs;
			pCurPart->m_vecPos += pCurPart->m_vecVel * FLoop_fPreviousLoopSecs;
		}
	}
	//
	////
#endif
}

// =============================================================================================================

void CHudParticle::Draw(f32 fXSoupOffset, CFColorRGB rgbSoupColor)
{
	u32 uPartIdx;
	CHudParticle *pCurPart;

//	static FDrawVtx_t avtxArray[(uDynamicParticlePoolSize + uHealthBarParticlePoolSize + uClusterParticlePoolSize) * 6];
	FDrawVtx_t *pavtxArray = fvtxpool_GetArray((uDynamicParticlePoolSize + uHealthBarParticlePoolSize) * 6);
	FASSERT(pavtxArray != NULL);

	u32 uCurVtxArrayIdx = 0;

	fdraw_SetTexture(m_ptexCur);
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);

#if 0
	//// Draw the gravity particles.
	//
	for(uPartIdx = 0; uPartIdx < uDynamicParticlePoolSize; ++uPartIdx)
	{
		pCurPart = &(CHudParticle_aoPool[uPartIdx]);
		if(pCurPart->m_bInUse)
		{
			f32 fFactor = pCurPart->m_fAge / pCurPart->m_fLifeTime;
			f32 fOMFactor = 1.0f - fFactor;

			CFColorRGB rgbCur, rgbTemp;
			rgbCur.White();
			rgbCur *= fOMFactor;
			rgbTemp = m_rgbWashColor;
			rgbTemp *= fFactor;
			rgbCur += rgbTemp;

			f32 fAlpha = 0.5f * (1.0f - fFactor);

			CFColorRGBA rgbaCur = rgbSoupColor;
			rgbaCur.Set(rgbCur, fAlpha);

//			pavtxArray[uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[uCurVtxArrayIdx].ColorRGBA = rgbaCur;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x - pCurPart->m_fSize, pCurPart->m_vecPos.y + pCurPart->m_fSize, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaCur;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x - pCurPart->m_fSize, pCurPart->m_vecPos.y - pCurPart->m_fSize, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 1.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaCur;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x + pCurPart->m_fSize, pCurPart->m_vecPos.y + pCurPart->m_fSize, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 0.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaCur;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x - pCurPart->m_fSize, pCurPart->m_vecPos.y - pCurPart->m_fSize, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 1.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaCur;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x + pCurPart->m_fSize, pCurPart->m_vecPos.y + pCurPart->m_fSize, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 0.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaCur;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x + pCurPart->m_fSize, pCurPart->m_vecPos.y - pCurPart->m_fSize, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 1.0f);
			
			++uCurVtxArrayIdx;
		}
	}
	//
	////

	//// Draw the soup particles.
	//
	for(uPartIdx = 0; uPartIdx < uHealthBarParticlePoolSize; ++uPartIdx)
	{
		pCurPart = &(CHudParticle_aoSoupPool[uPartIdx]);
		if(pCurPart->m_bInUse)
		{
			f32 fFactor = 0.7f;
			f32 fOMFactor = 1.0f - fFactor;

			f32 fSize = pCurPart->m_fSize;

			f32 fX1 = pCurPart->m_vecPos.x - fSize + fXSoupOffset;
			f32 fX2 = fX1 + 2.0f * fSize;
			f32 fY1 = pCurPart->m_vecPos.y + fSize;
			f32 fY2 = fY1 - 2.0f * fSize;

			CFColorRGBA rgbaSoupColor;
			rgbaSoupColor.ColorRGB = rgbSoupColor;
			rgbaSoupColor.fAlpha = 0.5f * (1.0f - fFactor);

//			pavtxArray[uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[uCurVtxArrayIdx].ColorRGBA = rgbaSoupColor;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(fX1, fY1, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaSoupColor;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(fX1, fY2, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 1.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaSoupColor;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(fX2, fY1, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 0.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaSoupColor;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(fX1, fY2, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 1.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaSoupColor;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(fX2, fY1, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 0.0f);

//			pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fOMFactor, fOMFactor, 1.0f, 0.5f * (1.0f - fFactor));
			pavtxArray[++uCurVtxArrayIdx].ColorRGBA = rgbaSoupColor;
			pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(fX2, fY2, 0.0f);
			pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 1.0f);
			
			++uCurVtxArrayIdx;
		}
	}
	//
	////

	//// Draw the cluster particles.
	//
	if(m_ePCState != PCSTATE_IDLE)
	{
		m_fPulsateTimer += FLoop_fPreviousLoopSecs;
		for(uPartIdx = 0; uPartIdx < uClusterParticlePoolSize; ++uPartIdx)
		{
			pCurPart = &(CHudParticle_aoClusterPool[uPartIdx]);
			if(pCurPart->m_bInUse)
			{
				f32 fAlpha;
				switch(m_ePCState)
				{
					case PCSTATE_APPEARING:
					{
						fAlpha = m_fPCCounter / fClusterFadeInTime;
						break;
					}
					case PCSTATE_FLOATING:
					{
						fAlpha = 1.0f;
						break;
					}
					case PCSTATE_DISAPPEARING:
					{
						fAlpha = 1.0f - m_fPCCounter / fClusterFadeOutTime;
						break;
					}
					default:
					{
						FASSERT_NOW;
					}
				}
				f32 fPulsate = 0.25f * fmath_Sin(m_fPulsateTimer + pCurPart->m_fPulsateOfs) + 0.75f;
				fAlpha *= ((fmath_RandomFloat() * 0.2f) + 0.8f);
				fAlpha *= 0.8f;

				pavtxArray[uCurVtxArrayIdx].ColorRGBA.Set(fPulsate, fPulsate, 1.0f, fAlpha);
				pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x - pCurPart->m_fSize, pCurPart->m_vecPos.y + pCurPart->m_fSize, 0.0f);
				pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

				pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fPulsate, fPulsate, 1.0f, fAlpha);
				pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x - pCurPart->m_fSize, pCurPart->m_vecPos.y - pCurPart->m_fSize, 0.0f);
				pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 1.0f);

				pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fPulsate, fPulsate, 1.0f, fAlpha);
				pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x + pCurPart->m_fSize, pCurPart->m_vecPos.y + pCurPart->m_fSize, 0.0f);
				pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 0.0f);

				pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fPulsate, fPulsate, 1.0f, fAlpha);
				pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x - pCurPart->m_fSize, pCurPart->m_vecPos.y - pCurPart->m_fSize, 0.0f);
				pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 1.0f);

				pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fPulsate, fPulsate, 1.0f, fAlpha);
				pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x + pCurPart->m_fSize, pCurPart->m_vecPos.y + pCurPart->m_fSize, 0.0f);
				pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 0.0f);

				pavtxArray[++uCurVtxArrayIdx].ColorRGBA.Set(fPulsate, fPulsate, 1.0f, fAlpha);
				pavtxArray[uCurVtxArrayIdx].Pos_MS.Set(pCurPart->m_vecPos.x + pCurPart->m_fSize, pCurPart->m_vecPos.y - pCurPart->m_fSize, 0.0f);
				pavtxArray[uCurVtxArrayIdx].ST.Set(1.0f, 1.0f);
			
				++uCurVtxArrayIdx;
			}
		}

		pavtxArray[uCurVtxArrayIdx].ColorRGBA.OpaqueWhite();
		pavtxArray[uCurVtxArrayIdx].Pos_MS = m_vecClusterPoint;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.y += 0.1f;
		pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

		pavtxArray[++uCurVtxArrayIdx].ColorRGBA.OpaqueWhite();
		pavtxArray[uCurVtxArrayIdx].Pos_MS = m_vecClusterPoint;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.y -= 0.1f;
		pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

		pavtxArray[++uCurVtxArrayIdx].ColorRGBA.OpaqueWhite();
		pavtxArray[uCurVtxArrayIdx].Pos_MS = m_vecClusterPoint;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.y -= 0.1f;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.x += 0.01f;
		pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

		pavtxArray[++uCurVtxArrayIdx].ColorRGBA.OpaqueWhite();
		pavtxArray[uCurVtxArrayIdx].Pos_MS = m_vecClusterPoint;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.x += 0.1f;
		pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

		pavtxArray[++uCurVtxArrayIdx].ColorRGBA.OpaqueWhite();
		pavtxArray[uCurVtxArrayIdx].Pos_MS = m_vecClusterPoint;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.x -= 0.1f;
		pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

		pavtxArray[++uCurVtxArrayIdx].ColorRGBA.OpaqueWhite();
		pavtxArray[uCurVtxArrayIdx].Pos_MS = m_vecClusterPoint;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.x -= 0.1f;
		pavtxArray[uCurVtxArrayIdx].Pos_MS.y += 0.01f;
		pavtxArray[uCurVtxArrayIdx].ST.Set(0.0f, 0.0f);

		++uCurVtxArrayIdx;
	}
	//
	////
#endif
	FASSERT(uCurVtxArrayIdx <= (uDynamicParticlePoolSize + uHealthBarParticlePoolSize) * 6);

	if(uCurVtxArrayIdx != 0)
		fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, pavtxArray, uCurVtxArrayIdx);

	fvtxpool_ReturnArray(pavtxArray);
}
#endif

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

CWasherDisplay::CWasherDisplay()
{
	m_pMeshInst = NULL;
	m_pMesh = NULL;
	m_pView = NULL;
}

// =============================================================================================================

CWasherDisplay::~CWasherDisplay()
{
	if(m_pMeshInst != NULL)
	{
		fdelete( m_pMeshInst );
		m_pMeshInst = NULL;
	}
	m_pMesh = NULL;
}

// =============================================================================================================

BOOL CWasherDisplay::Create()
{
	FResFrame_t ResFrame = fres_GetFrame();
	FTextArea_t oTextArea;
	FMeshInit_t oMeshInit;

	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.ohFont = '1';
	oTextArea.fUpperLeftY = fWasherTextY;
	oTextArea.fLowerRightY = fWasherTextY + fWasherHeight * 0.30f;
	oTextArea.oColorBackground.OpaqueBlack();
	oTextArea.fBorderThicknessX = 0.0f;
	oTextArea.fBorderThicknessY = 0.0f;
	oTextArea.oColorBorder.TransparentBlack();
	oTextArea.bVisible = FALSE;

	m_hText = ftext_Create(&oTextArea);

	m_eState = WASHERSTATE_OFF;

	m_pView = fviewport_Create();
	if (!m_pView)
	{
		goto _ExitJustin_InitSysWithError;
	}

	FASSERT(m_pMeshInst == NULL);
	m_pMeshInst = fnew CFMeshInst;
	if (m_pMeshInst == NULL)
	{
		goto _ExitJustin_InitSysWithError;
	}

	m_pMesh = (FMesh_t *)(fresload_Load(FMESH_RESTYPE, "gp_wwasher1"));
	if(m_pMesh == NULL)
	{
		DEVPRINTF("CWasherDisplay::Init() : Could not load washer mesh.\n");
	}

	// NKM - Do this here rather than each frame in the display of a spinning washer.
	oMeshInit.fCullDist = FMATH_MAX_FLOAT;
	oMeshInit.Mtx.Identity();
	oMeshInit.nFlags = 0;
	FASSERT(m_pMesh != NULL);
	oMeshInit.pMesh = m_pMesh;

	m_pMeshInst->Init(&oMeshInit);

	return TRUE;

_ExitJustin_InitSysWithError:

	DEVPRINTF( "HUD2::WasherDisplay: Could not init system.\n" );
	if (m_pMeshInst)
		fdelete( m_pMeshInst );
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}

void CWasherDisplay::Destroy()
{
	if (m_pMeshInst)
		fdelete( m_pMeshInst );
	m_pMeshInst = NULL;
	m_pMesh = NULL;
	m_pView = NULL;
}


// =============================================================================================================

void CWasherDisplay::Draw3D()
{
	// Sanity check
	FASSERT( m_eState <= WASHERSTATE_SLIDINGOFF );

	if (m_eState == WASHERSTATE_OFF)
		return;

	// NKM - This is VERY, VERY, VERY bad
/*	FMeshInit_t oMeshInit;
	oMeshInit.fCullDist = FMATH_MAX_FLOAT;
	oMeshInit.Mtx.Identity();
	oMeshInit.nFlags = 0;
	FASSERT(m_pMesh != NULL);
	oMeshInit.pMesh = m_pMesh;

	CFMtx43 mtxTemp;

	m_pMeshInst->Init(&oMeshInit);*/

	CFColorRGB oColorRGB;
	f32 fIntensity;

	fmesh_Ambient_Get(&oColorRGB, &fIntensity);
	fmesh_Ambient_Set(1.0f, 1.0f, 1.0f, 0.75f);
//	fmesh_Ambient_Set(1.0f, 1.0f, 1.0f, 0.01f);

//	FMeshLightFrame_t hLightFrame = fmesh_StartBlackFrame();

	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.RotateY(m_fThetaY);
	CFMtx43A::m_Temp.m_vPos.Set(0.0f, 0.0f, 10.0f);
	CFMtx43A::m_Temp.m_vX.Mul( 4.0f );
	CFMtx43A::m_Temp.m_vY.Mul( 4.0f );
	CFMtx43A::m_Temp.m_vZ.Mul( 4.0f );

	fviewport_SetActive(m_pView);

//	flight_InitDirLight(&m_oLight, -1.0f, 0.0f, 0.0f);

	m_pMeshInst->m_Xfm.BuildFromMtx(CFMtx43A::m_Temp);
	m_pMeshInst->Draw(FVIEWPORT_PLANESMASK_NONE);

//	fmesh_ReleaseLightFrame(hLightFrame);

	fmesh_Ambient_Set(&oColorRGB, fIntensity);
}

// =============================================================================================================

void CWasherDisplay::Draw2D()
{
	// Sanity check
	FASSERT( m_eState <= WASHERSTATE_SLIDINGOFF );

	if (m_eState == WASHERSTATE_OFF)
		return;

	//Hud2_XFormPrintf( m_hText, "%d", m_nWasherCountToDraw );
	ftext_Printf( m_hText, "%d", m_nWasherCountToDraw );
}

// =============================================================================================================

void CWasherDisplay::SetUL(CFVec3 *vecUL, FViewport_t* pRefVP)
{
	// We need to build our viewport inside the reference viewport
	u32 uX1 = pRefVP->nScreenLeftX + fmath_FloatToU32((vecUL->x + 1.0f) * pRefVP->HalfRes.x);
	u32 uY1 = pRefVP->nScreenTopY + fmath_FloatToU32((0.75f - vecUL->y) * (1.0f / 1.5f) * pRefVP->Res.y);
	u32 uWidth = fmath_FloatToU32(fWasherWidth * pRefVP->HalfRes.x);
	u32 uHeight = fmath_FloatToU32(fWasherHeight * pRefVP->HalfRes.x);	// Make it more or less square, regardless of viewport proportions

	fviewport_InitPersp(m_pView, /*FMATH_QUARTER_PI*/ FMATH_PI * (1.0f / 6.0f), 0.1f, 1000.0f, uX1, uY1, uWidth, uHeight);
	if(vecUL->x < 1.0f)
	{
		ftext_GetAttributes(m_hText)->fUpperLeftX = JCXToFTextX(vecUL->x + fWasherWidth);
		ftext_GetAttributes(m_hText)->fLowerRightX = JCXToFTextX(vecUL->x + fWasherWidth) + 0.3f;
	}
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

s32 CHudItemInst::m_nActiveCount = 0;

// =============================================================================================================

CHudItemInst::CHudItemInst()
{
	m_poItem = NULL;
	m_pCollectableType = NULL;
	m_eItemState = ITEMSTATE_EMPTY;

	// These items don't really need to be initialized here, but just to keep
	// them from having garbage values...
	m_fScale = 1.0f;
	m_fConstant = 1.0f;
	m_fTimeCounter = 0.0f;
	m_fTotalTime = 0.0f;
	m_eItemType = ITEMTYPE_ENERGY;
}

// =============================================================================================================

CHudItemInst::~CHudItemInst()
{
	FASSERT(m_poItem == NULL);
}

// =============================================================================================================

BOOL CHudItemInst::InitFromMI(CCollectableType *pCollectableType)
{
	FASSERT( pCollectableType );
	FASSERT(m_nActiveCount >= 0);

	if( m_pCollectableType ) {
		Clear();
	}

	m_poItem = pCollectableType->GetWorldMesh();

	if( m_poItem == NULL ) {
		DEVPRINTF("CHudItemInst::InitFromMI() : Failed to construct needed CFMeshInst.\n");
		m_eItemState = ITEMSTATE_EMPTY;
		return FALSE;
	}

	m_pCollectableType = pCollectableType;

	/*if (m_poItem == NULL)
		m_poItem = fnew CFMeshInst;
	if(m_poItem == NULL)
	{
		DEVPRINTF("CHudItemInst::InitFromMI() : Failed to construct needed CFMeshInst.\n");
		m_eItemState = ITEMSTATE_EMPTY;
		return FALSE;
	}

	FMeshInit_t oMeshInit;
	oMeshInit.fCullDist = FMATH_MAX_FLOAT;
	oMeshInit.Mtx.Identity();
	// TODO : Copy over the flags of the WorldMesh.
	oMeshInit.nFlags = 0;
	oMeshInit.pMesh = poMI->m_pMesh;
	m_poItem->Init(&oMeshInit);*/

	m_vecStartPos_CS.Set(0.0f, -4.0f, 6.0f);

	m_eItemState = ITEMSTATE_JUMPINGUP;
	m_fTimeCounter = 0.0f;
	m_fTotalTime = 0.0f;
	m_fConstant = fmath_InvSqrt(m_vecEndPos_CS.y - m_vecStartPos_CS.y);

	++m_nActiveCount;

	return TRUE;
}

// =============================================================================================================

void CHudItemInst::CalcMtx(f32 fThetaY, CFVec3 *vecPos_CS)
{
	FASSERT(m_nActiveCount >= 0);
	FASSERT( m_poItem );

	/*CFMtx43 mtxTemp;//, mtxTemp2;

	mtxTemp.Identity();
	mtxTemp.SetRotationY(fThetaY);
	mtxTemp.m_vPos = *vecPos_CS;*/
	// NKM
	CFVec3A Center = m_poItem->m_BoundSphere_MS.m_Pos;

	Center.Mul( -1.0f );

	CFMtx43A::m_XlatRotY.m_vPos.Zero();
	
	if( !( m_poItem->m_nFlags & ( FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z ) ) ) {
		CFMtx43A::m_XlatRotY.SetRotationY( fThetaY );
	}

	CFMtx43A::m_XlatRotY.MulPoint( Center, Center );
	Center.Mul( m_fScale );
    CFMtx43A::m_XlatRotY.m_vPos = *vecPos_CS;
	CFMtx43A::m_XlatRotY.m_vPos.Add( Center );

	m_poItem->m_Xfm.BuildFromMtx(CFMtx43A::m_XlatRotY, m_fScale);
}

// =============================================================================================================

void CHudItemInst::Draw()
{
	FASSERT(m_nActiveCount >= 0);

	if(m_eItemState != ITEMSTATE_EMPTY)
	{
		m_poItem->ResetLightList();
		m_poItem->ConsiderLightForRender( &CHud2::m_HUDDirLight );
		m_poItem->Draw( FVIEWPORT_PLANESMASK_ALL );
	}
}

// =============================================================================================================

void CHudItemInst::Empty()
{
	FASSERT(m_nActiveCount >= 1);

	m_eItemState = ITEMSTATE_EMPTY;

	--m_nActiveCount;

	Clear();
}

// =============================================================================================================

void CHudItemInst::Clear()
{
	if(m_poItem != NULL)
	{
		//fdelete( m_poItem );
		FASSERT( m_pCollectableType );
		
		m_pCollectableType->ReturnWorldMesh( m_poItem );
		m_poItem = NULL;
		m_pCollectableType = NULL;
	}

	if(m_eItemState != ITEMSTATE_EMPTY)
	{
		Empty();
	}
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

BOOL			CHudBatteryBarSystem::ms_bSystemInitted = FALSE;
CFTexInst		CHudBatteryBarSystem::ms_atexParticle[2];
CFTexInst		CHudBatteryBarSystem::ms_texBatteryNormal;
FDrawVtx_t		CHudBatteryBarSystem::ms_aavtxBatteryBar[2][24];
CFMeshInst*		CHudBatteryBarSystem::ms_pmeshBP = NULL;

// =============================================================================================================

BOOL CHudBatteryBarSystem::InitSystem()
{
	FASSERT(!ms_bSystemInitted);

	// DFS TODO: Move this?
//	CHudParticle::InitSystem();

#if 0
	//// Load and set up the particle textures for the particle system.
	//
	// TODO: We only need one spark texture.
	FTexDef_t *aptexParticleSystem[2];
	aptexParticleSystem[0] = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "TFH2spark01"));
	if(aptexParticleSystem[0] == NULL)
	{
		DEVPRINTF("CHudBatteryBarSystem::InitSystem() : Could not load blue spark texture.\n");
	}
	aptexParticleSystem[1] = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "TFH2spark01"));
	if(aptexParticleSystem[1] == NULL)
	{
		DEVPRINTF("CHudBatteryBarSystem::InitSystem() : Could not load red spark texture.\n");
	}

//	CHudParticle::SetParticleTextures(aptexParticleSystem[0], aptexParticleSystem[1]);
	//
	////
#endif

	//// Load the texture for the battery bar.
	//
	ms_texBatteryNormal.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2hudall1")) );
	if(ms_texBatteryNormal.GetTexDef() == NULL)
	{
		DEVPRINTF("CHudBatteryBarSystem::InitSystem() : Could not load HUD battery texture.\n");
	}
	//
	////
/*
	//// Initialize the plasma in the health bar.
	//
	CFVec2 vecSoupUL, vecSoupLR;
	vecSoupUL.Set(fBatteryMeterLE, fBatteryMeterTE - fHealthBarOfsY);
	// TODO: This 1.0f should be Blink's starting energy.
	vecSoupLR.Set(fBatteryMeterLE + fBatteryMeterSize * 1.0f, fBatteryMeterTE - fHealthBarOfsY - fHealthBarHeight);

	CHudParticle::SetParticleSoupRegion(&vecSoupUL, &vecSoupLR);
	CHudParticle::AllocParticleSoup(0.008f, 80);
	CHudParticle::AllocParticleSoup(0.010f, 60);
	//
	////
*/
	//// Setup the vertex array for the battery bar.
	//

	u32 uHudType;
	for(uHudType = 0; uHudType < 2; ++uHudType)
	{
		ms_aavtxBatteryBar[uHudType][0].ColorRGBA.OpaqueBlue();
		ms_aavtxBatteryBar[uHudType][0].Pos_MS.Set(fBatteryMeterLE, fBatteryMeterTE, 0.0f);
		ms_aavtxBatteryBar[uHudType][0].ST.Set(0.25f, 0.0f + 0.125f * uHudType);

		ms_aavtxBatteryBar[uHudType][1].ColorRGBA.OpaqueBlue();
		ms_aavtxBatteryBar[uHudType][1].Pos_MS.Set(fBatteryMeterLE + fBatteryMeterSize * 1.27f, fBatteryMeterTE, 0.0f);
		ms_aavtxBatteryBar[uHudType][1].ST.Set(0.44f, 0.0f + 0.125f * uHudType);

		ms_aavtxBatteryBar[uHudType][2].ColorRGBA.OpaqueBlue();
		ms_aavtxBatteryBar[uHudType][2].Pos_MS.Set(fBatteryMeterLE + fBatteryMeterSize * 1.27f, fBatteryMeterTE - fBatteryMeterSize, 0.0f);
		ms_aavtxBatteryBar[uHudType][2].ST.Set(0.44f, 0.125f + 0.125f * uHudType);

		ms_aavtxBatteryBar[uHudType][3].ColorRGBA.OpaqueBlue();
		ms_aavtxBatteryBar[uHudType][3].Pos_MS.Set(fBatteryMeterLE, fBatteryMeterTE - fBatteryMeterSize, 0.0f);
		ms_aavtxBatteryBar[uHudType][3].ST.Set(0.25f, 0.125f + 0.125f * uHudType);

		// Set up the last five batteries to be like the first one.
		u32 uBattIdx, uVtxIdx;
		for(uBattIdx = 1; uBattIdx < 6; ++uBattIdx)
		{
			for(uVtxIdx = 0; uVtxIdx < 4; ++uVtxIdx)
			{
				ms_aavtxBatteryBar[uHudType][uBattIdx * 4 + uVtxIdx] = ms_aavtxBatteryBar[uHudType][(uBattIdx - 1) * 4 + uVtxIdx];
				ms_aavtxBatteryBar[uHudType][uBattIdx * 4 + uVtxIdx].Pos_MS.x += (fBatteryMeterSize * 1.125f);
			}
		}
		//
		////
	}

	//// Set up the mesh for the battery pickup.
	//
	FMesh_t *pMesh = (FMesh_t *)(fresload_Load(FMESH_RESTYPE, /*"gp_wwasher1"*/"gp_1battery"));
	if(pMesh == NULL)
	{
		DEVPRINTF("CBatteryBarSystem::InitSystem() : Could not load battery mesh.\n");
	}
	else
	{
		ms_pmeshBP = fnew CFMeshInst;
		if(ms_pmeshBP == NULL)
		{
			DEVPRINTF("CBatteryBarSystem::InitSystem() : Could not create battery CFMeshInst.\n");
		}
		else
		{
			FMeshInit_t oMeshInit;

			oMeshInit.fCullDist = FMATH_MAX_FLOAT;
			oMeshInit.Mtx.Identity();
			oMeshInit.nFlags = 0;
			oMeshInit.pMesh = pMesh;

			ms_pmeshBP->Init(&oMeshInit);
		}
	}
	//
	////

	ms_bSystemInitted = TRUE;

	return(TRUE);
}

// =============================================================================================================

void CHudBatteryBarSystem::InitLevel()
{
}

// ============================================================================
BOOL CHudBatteryBarSystem::Create()
{
	m_bFirstFrame = TRUE;
	m_eEBState = EBSTATE_IDLE;
	m_eBPState = BPSTATEN_IDLE;
	return TRUE;
}

// ============================================================================
void CHudBatteryBarSystem::Destroy()
{
}

// =============================================================================================================

void CHudBatteryBarSystem::StartBatteryPickup()
{
//	m_vecLastBPPos.Set(0.0f, 0.0f, 0.0f);
	m_vecLastBPPos = CHudBatterySystem_avecBPCurvePoints[0];
//	m_vecLastBPVel.Set(0.0f, 0.0f, 0.0f);

//	m_fBPThetaZ = 0.0f;
//	m_fBPOmegaZ = 0.0f;

	m_fBPThetaY = 0.0f;
	m_fBPThetaZ = 0.0f;

	m_afBPTimer[0] = 0.0f;
	m_afBPTimer[1] = 0.0f;
	m_eBPState = BPSTATEN_PAUSING;
}

// =============================================================================================================

void CHudBatteryBarSystem::SetHudMode(HudMode_e eNewHudMode)
{
	m_eCurHudMode = eNewHudMode;
//	CHudParticle::SetHudMode(eNewHudMode);
	switch(eNewHudMode)
	{
		case HUDMODE_GLITCH:
		case HUDMODE_SLOSH:
		case HUDMODE_KRUNK:
		case HUDMODE_MOZER:
		{
			m_rgbaEnergyBarNormal.Blue();
			m_rgbaEnergyBarNormal.fAlpha = 0.5f;
			break;
		}
		case HUDMODE_MIL:
		{
			m_rgbaEnergyBarNormal.Red();
			m_rgbaEnergyBarNormal.fAlpha = 0.5f;
			break;
		}
	}
	m_rgbaEnergyBarCur = m_rgbaEnergyBarNormal;
	// Set up the two CFColorRGBAs.
}

// =============================================================================================================

void CHudBatteryBarSystem::Work(CInventory *pInventory, f32 fBotHealth)
{
	// Update state/position of pickup battery and battery strip if applicable.

	f32 fMyLastLoop = FLoop_fPreviousLoopSecs;// * 0.1f;

	//// State transitions for the pickup battery.
	//
	switch(m_eBPState)
	{
		case BPSTATEN_IDLE:
		{
			break;
		}
		case BPSTATEN_PAUSING:
		{
			m_afBPTimer[0] += fMyLastLoop;
			m_afBPTimer[1] += fMyLastLoop;
			if(m_afBPTimer[1] >= CHudBatterySystem_fBPPauseDur)
			{
				m_afBPTimer[1] = 0.0f;

				f32 fControl = (f32)(pInventory->m_uNumBatteries - 3) * 0.5f;

				m_fBPFlyDurFactor = FMATH_FPOT(fControl, 1.00f, 0.97f);

				m_eBPState = BPSTATEN_FLYING;
			}

			break;
		}
		case BPSTATEN_FLYING:
		{
			m_afBPTimer[0] += fMyLastLoop;
			m_afBPTimer[1] += fMyLastLoop;
			if(m_afBPTimer[1] >= CHudBatterySystem_fBPFlyDur * m_fBPFlyDurFactor)
			{
				m_fXOffsetPos = 0.0f;//m_vecLastBPPos.x - CHudBatterySystem_fHalfBPIconHeight - (fBatteryMeterLE + fBatteryMeterSize * pInventory->m_uNumBatteries);
				m_fXOffsetVel = -3.0f;//m_vecLastBPVel.x;

				CFVec2 vecUL, vecBR;
				vecUL.Set(fBatteryMeterLE + fBatteryMeterSize * pInventory->m_uNumBatteries - 0.05f, fBatteryMeterTE - fHealthBarOfsY);
				vecBR.Set(fBatteryMeterLE + fBatteryMeterSize * pInventory->m_uNumBatteries + 0.05f, fBatteryMeterTE - fHealthBarOfsY - fHealthBarHeight);
//				CHudParticle::AllocParticlePatch(&vecUL, &vecBR, 0.0125f, 0.8f, 0.1f, 20);
//				CHudParticle::AllocParticlePatch(&vecUL, &vecBR, 0.00625f, 1.6f, 0.1f, 30);

				++pInventory->m_uNumBatteries;
				// REPAIR:
//				Bot_pPlayerBot->fUnitHealth = (f32)(pInventory->m_uNumBatteries) * (1.0f / 3.0f);

				m_afBPTimer[1] = 0.0f;
				m_eBPState = BPSTATEN_STABILIZING;
			}

			break;
		}
		case BPSTATEN_STABILIZING:
		{
			m_afBPTimer[1] += fMyLastLoop;
			if(m_afBPTimer[1] >= CHudBatterySystem_fBPStabilizeDur)
			{
				m_eBPState = BPSTATEN_IDLE;
			}
			break;
		}
	}
	//
	////

	//// Work for the pickup battery.
	//
	switch(m_eBPState)
	{
		case BPSTATEN_IDLE:
		{
			break;
		}
		case BPSTATEN_PAUSING:
		{
			m_fBPThetaY = m_afBPTimer[0] * CHudBatterySystem_fBPOmegaY;
			break;
		}
		case BPSTATEN_FLYING:
		{
			f32 fT = m_afBPTimer[1] * CHudBatterySystem_fOOBPFlyDur;
			FASSERT_UNIT_FLOAT(fT);

			f32 fOMT = 1.0f - fT;

			CFVec3 vecPos, vecTemp;

			f32 fCoeff;
			u32 uTerm, uCoeff;
			vecPos.Set(0.0f, 0.0f, 0.0f);
			for(uTerm = 0; uTerm < 6; ++uTerm)
			{
				fCoeff = 1.0f;
				for(uCoeff = 0; uCoeff < uTerm; ++uCoeff)
				{
					fCoeff *= fT;
				}
				for(; uCoeff < 5; ++uCoeff)
				{
					fCoeff *= fOMT;
				}
				fCoeff *= CHudBatterySystem_afBPCoeff[uTerm];

				vecTemp = CHudBatterySystem_avecBPCurvePoints[uTerm];
				vecTemp *= fCoeff;

				vecPos += vecTemp;
			}

			m_vecLastBPPos = vecPos * 11.0f;

			m_fBPThetaY = m_afBPTimer[0] * CHudBatterySystem_fBPOmegaY;
			m_fBPThetaZ = m_afBPTimer[1] * CHudBatterySystem_fOOBPFlyDur * -FMATH_HALF_PI;

			break;
		}
		case BPSTATEN_STABILIZING:
		{
			f32 fForce = -100.0f * m_fXOffsetPos + -6.0f * m_fXOffsetVel;

			m_fXOffsetVel += fForce * fMyLastLoop;
			m_fXOffsetPos += m_fXOffsetVel * fMyLastLoop;

			break;
		}
	}
	//
	////

#if 0
	//// Initialize the plasma in the health bar.
	//
	// DFS -- Move this to "Create"
	if(m_bFirstFrame)
	{
		CFVec2 vecSoupUL, vecSoupLR;
		// JUSTIN: Fix these positions.
		vecSoupUL.Set(fBatteryMeterLE + _fEnergyOffsetX, fBatteryMeterTE - fHealthBarOfsY);
		vecSoupLR.Set(fBatteryMeterLE + fBatteryMeterSize * fBotHealth + _fEnergyOffsetX, fBatteryMeterTE - fHealthBarOfsY - fHealthBarHeight);

		CHudParticle::SetParticleSoupRegion(&vecSoupUL, &vecSoupLR);
		CHudParticle::AllocParticleSoup(0.008f, 80);
		CHudParticle::AllocParticleSoup(0.010f, 60);
	}
	//
	////
#endif

	//// Check for energy gain/loss.
	//
//	BOOL bHealthHasChanged = FALSE;
	if(!m_bFirstFrame)
	{
		if(fBotHealth < m_fLastHealth)
		{
#if 0
			// Handle energy-loss effects.
			CFVec2 vecUL, vecBR;
			vecUL.Set(fBatteryMeterLE + fBatteryMeterSize * fBotHealth, fBatteryMeterTE - fHealthBarOfsY);
			vecBR.Set(fBatteryMeterLE + fBatteryMeterSize * m_fLastHealth, fBatteryMeterTE - fHealthBarOfsY - fHealthBarHeight);
			CHudParticle::AllocParticlePatch(&vecUL, &vecBR, 0.0125f, 0.8f, 0.1f, fmath_FloatToU32(fHealthBarParticleDensity * (m_fLastHealth - fBotHealth)));
			CHudParticle::AllocParticlePatch(&vecUL, &vecBR, 0.00625f, 1.6f, 0.1f, fmath_FloatToU32(fHealthBarParticleDensity * (m_fLastHealth - fBotHealth)));

			bHealthHasChanged = TRUE;
#endif
		}
		else if(fBotHealth > m_fLastHealth)
		{
			// Handle energy-gain effects.
			m_fEnergyGainTimer = 0.0f;
			m_eEBState = EBSTATE_GAINING;

			// We will limit the rate at which we will reflect health gain (health loss is instantaneous).
			fBotHealth = FMATH_MIN(m_fLastHealth + CHudBatterySystem_fEnergyChargeRate * fMyLastLoop, fBotHealth);

//			bHealthHasChanged = TRUE;
		}

#if 0
		////  Reset the particle soup to reflect any change in health.
		//
		if(bHealthHasChanged)
		{
			CFVec2 vecUL, vecBR;
			vecUL.Set(fBatteryMeterLE, fBatteryMeterTE - fHealthBarOfsY);
			vecBR.Set(fBatteryMeterLE + fBatteryMeterSize * fBotHealth, fBatteryMeterTE - fHealthBarOfsY - fHealthBarHeight);
			CHudParticle::ResetParticleSoup(&vecUL, &vecBR, 0.0124f, fmath_FloatToU32(fHealthBarParticleDensity * fBotHealth));
		}
		//
		////
#endif
	}
	//
	////
	m_fLastHealth = fBotHealth;

	//// Calculate the current color to use.
	//
	// This should get changed to not be a divide.
	FASSERT(fBotHealth <= (f32)(pInventory->m_uNumBatteries));
	f32 fUnitHealth = fBotHealth / (f32)(pInventory->m_uNumBatteries);
	m_rgbaEnergyBarCur.fBlue = 0.0f;
	m_rgbaEnergyBarCur.fAlpha = 0.5f;
	if(fUnitHealth > 0.5f)
	{
		m_rgbaEnergyBarCur.fRed = 2.0f - 2.0f * fUnitHealth;
		m_rgbaEnergyBarCur.fGreen = 1.0f;
	}
	else
	{
		m_rgbaEnergyBarCur.fRed = 1.0f;
		m_rgbaEnergyBarCur.fGreen = 2.0f * fUnitHealth;
	}
	//
	////

	// Set up the background transparency level.

	// Call the Particle Work.
//	CHudParticle::Work();

	// Pickup battery state: Bouncing up, bouncing and rotating, moving left, stabilizing 
	m_bFirstFrame = FALSE;
}

// =============================================================================================================

//void CHudBatteryBarSystem::Draw(CInventory *pInventory)
void CHudBatteryBarSystem::Draw( u32 uNumBatteries, f32 *pfVal/*=NULL*/, const CFColorRGBA *pStartColor/*=NULL*/, const CFColorRGBA *pEndColor/*=NULL*/ ) {

	f32 fVal;
	CFColorRGBA color;
	if( (pfVal != NULL) ) {		// this is our override state
		f32 fUnitVal;
		fVal	= *pfVal;
		fUnitVal = fmath_Div( fVal, (f32)uNumBatteries);
		color.fRed		= pStartColor->fRed + (pEndColor->fRed-pStartColor->fRed) * fUnitVal;
		color.fGreen	= pStartColor->fGreen + (pEndColor->fGreen-pStartColor->fGreen) * fUnitVal;
		color.fBlue		= pStartColor->fBlue + (pEndColor->fRed-pStartColor->fBlue) * fUnitVal;
		color.fAlpha	= pStartColor->fAlpha + (pEndColor->fAlpha-pStartColor->fAlpha) * fUnitVal;
	} else {
		fVal = m_fLastHealth;
		color = m_rgbaEnergyBarCur;
	}

	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECALTEX_AT);

	//// Draw battery strip according to offset.
	//
	fdraw_SetTexture(&ms_texBatteryNormal);
//	u32 uNumBatteries = pInventory->m_uNumBatteries;
	CFXfm xfmShake;
	xfmShake.BuildTranslation(m_fXOffsetPos, 0.0f, 0.0f);
	xfmShake.PushModel();
	u32 uHudModeIdx = m_eCurHudMode == HUDMODE_MIL ? 1 : 0;
	fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, ms_aavtxBatteryBar[uHudModeIdx], uNumBatteries * 4);
	CFXfm::PopModel();
	//
	////

#if 0
	if( pfVal == NULL ) {	// if we're not overriding
		CHudParticle::Draw(m_fXOffsetPos, color.ColorRGB);
	}
#endif

	FDrawVtx_t avtxEnergy[4];

	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);
	//// Draw the energy bar.
	//
	f32 fX1 = fBatteryMeterLE + m_fXOffsetPos + _fEnergyOffsetX - 0.005f;
	f32 fX2 = fX1 + fBatteryMeterSize * 1.108f * fVal;
	f32 fY1 = fBatteryMeterTE - fHealthBarOfsY;
	f32 fY2 = fY1 - fHealthBarHeight;

	avtxEnergy[0].ColorRGBA = color;
	avtxEnergy[0].Pos_MS.Set(fX1, fY1, 0.0f);

	avtxEnergy[1].ColorRGBA = color;
	avtxEnergy[1].Pos_MS.Set(fX2, fY1, 0.0f);

	avtxEnergy[2].ColorRGBA = color;
	avtxEnergy[2].Pos_MS.Set(fX2, fY2, 0.0f);

	avtxEnergy[3].ColorRGBA = color;
	avtxEnergy[3].Pos_MS.Set(fX1, fY2, 0.0f);

	fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, avtxEnergy, 4);
	//
	////
/*
	//// Draw the flight points.
	//
	FDrawVtx_t avtxFlight[6 * 6];
	u32 uCurPosIdx, uCurVtx = 0;
	for(uCurPosIdx = 0; uCurPosIdx < 6; ++uCurPosIdx)
	{
		f32 fX = CHudBatterySystem_avecBPCurvePoints[uCurPosIdx].x;
		f32 fY = CHudBatterySystem_avecBPCurvePoints[uCurPosIdx].y;
		f32 fSize = 0.02f;

		avtxFlight[uCurVtx].ColorRGBA.OpaqueGreen();
		avtxFlight[uCurVtx].Pos_MS.Set(fX - fSize, fY + fSize, 0.0f);
		avtxFlight[uCurVtx].ST.Set(0.0f, 0.0f);

		avtxFlight[++uCurVtx].ColorRGBA.OpaqueGreen();
		avtxFlight[uCurVtx].Pos_MS.Set(fX + fSize, fY + fSize, 0.0f);
		avtxFlight[uCurVtx].ST.Set(0.0f, 0.0f);

		avtxFlight[++uCurVtx].ColorRGBA.OpaqueGreen();
		avtxFlight[uCurVtx].Pos_MS.Set(fX - fSize, fY - fSize, 0.0f);
		avtxFlight[uCurVtx].ST.Set(0.0f, 0.0f);

		avtxFlight[++uCurVtx].ColorRGBA.OpaqueGreen();
		avtxFlight[uCurVtx].Pos_MS.Set(fX + fSize, fY + fSize, 0.0f);
		avtxFlight[uCurVtx].ST.Set(0.0f, 0.0f);

		avtxFlight[++uCurVtx].ColorRGBA.OpaqueGreen();
		avtxFlight[uCurVtx].Pos_MS.Set(fX - fSize, fY - fSize, 0.0f);
		avtxFlight[uCurVtx].ST.Set(0.0f, 0.0f);

		avtxFlight[++uCurVtx].ColorRGBA.OpaqueGreen();
		avtxFlight[uCurVtx].Pos_MS.Set(fX + fSize, fY - fSize, 0.0f);
		avtxFlight[uCurVtx].ST.Set(0.0f, 0.0f);

		++uCurVtx;
	}

	FASSERT(uCurVtx == 6 * 6);
	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, avtxFlight, uCurVtx);
	//
	////
*/
}

// =============================================================================================================

void CHudBatteryBarSystem::DrawMeshes(CInventory *pInventory)
{
	// It is assumed that the renderer has already been set to mesh rendering.
	if((m_eBPState != BPSTATEN_IDLE) && (m_eBPState != BPSTATEN_STABILIZING))
	{
		CFMtx43 mtxTemp;

		mtxTemp.Identity();
		mtxTemp.RotateY(m_fBPThetaY);
		mtxTemp.RotateZ(m_fBPThetaZ);
		mtxTemp.m_vPos = m_vecLastBPPos;//.Set(0.0f, 0.0f, 10.0f);
//		mtxTemp.m_vPosition.Set(0.0f, 0.0f, 0.0f);
		mtxTemp.m_vPos.z = 20.0f;
//		mtxTemp.m_vX *= 400.0f;
//		mtxTemp.m_vY *= 400.0f;
//		mtxTemp.m_vZ *= 400.0f;
//		mtxTemp.m_vPosition.Set(0.0f, -1.04f, 20.0f);

		ms_pmeshBP->m_Xfm.BuildFromMtx(mtxTemp);
		ms_pmeshBP->Draw(FVIEWPORT_PLANESMASK_NONE);
	}
}

// =============================================================================================================

void CHudBatteryBarSystem::EndLevel()
{
}

// =============================================================================================================

void CHudBatteryBarSystem::Shutdown()
{
	if(!ms_bSystemInitted)
	{
		return;
	}

	if(ms_pmeshBP != NULL)
	{
		fdelete( ms_pmeshBP );
		ms_pmeshBP = NULL;
	}

	ms_bSystemInitted = FALSE;
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

CFLight CHud2::m_HUDDirLight;

// Private static variables
BOOL				CHud2::_bSystemInitialized = FALSE;
BOOL				CHud2::_bLevelInitialized = FALSE;
FSndFx_FxHandle_t	CHud2::_hScrollClick = NULL;
FSndFx_FxHandle_t	CHud2::_hSelect = NULL;
CFTexInst			CHud2::_texInfoBox;
CFTexInst			CHud2::_texHud;
CFTexInst			CHud2::_texControls;
f32					CHud2::m_fAmmoAlertUnitCountdownTimer;
BOOL				CHud2::m_bAudioPause=TRUE;

// _pCurrentHud is used internally only to track the current hud during the
// radar callback.
static CHud2*		_pCurrentHud = NULL;

// CHud2 Static functions
CHud2* CHud2::GetCurrentHud(void)
{
	return &CPlayer::m_pCurrent->m_Hud;
}

CHud2* CHud2::GetHudForPlayer(s32 nPlayerIndex)
{
	FASSERT( nPlayerIndex >= 0 && nPlayerIndex < MAX_PLAYERS );
	return &Player_aPlayer[nPlayerIndex].m_Hud;
}

// ============================================================================
CHud2::CHud2()
{
	m_bDrawEnabled = TRUE;
	m_bWSEnabled = TRUE;
	m_uNumBatteries = 3;

	//// Item pickup related things.
	//
	// Set the final positions.
	m_aoHudItemInst[0].m_vecEndPos_CS.Set(0.0f, /*5.0f*/3.5f, 15.0f);
	m_aoHudItemInst[1].m_vecEndPos_CS.Set(3.0f, /*5.0f*/3.5f, 15.0f);
	m_aoHudItemInst[2].m_vecEndPos_CS.Set(-3.0f, /*5.0f*/3.5f, 15.0f);

	m_pviewItemPickup = NULL;
	m_pviewOrtho3d = NULL;
	m_pviewPerspective[0] = NULL;
	m_pviewPerspective[1] = NULL;
	
	
	////
	//
	m_bOverrideHealth			= FALSE;		
	m_pfHealthOverrideData		= FALSE;
	m_uHealthOverrideBatteries	= 0;
}

// ============================================================================
CHud2::~CHud2()
{
}

// ============================================================================
// Create is called each time a new level starts or a new player is
// created.
BOOL CHud2::Create(s32 nPlayerIndex)
{
	FTextArea_t oTextArea;

	//CPS 4.7.03 -->
	u32 uCurVtx;
	f32 fS1;					
	f32 fS2;
	f32 fT1;
	f32 fT2;
	f32 fExitBotLeft;
	FViewport_t* pVp = NULL;
	//<-- CPS 4.7.03

	m_HUDDirLight.InitDirLight( 1.0f, -1.0f, 1.0f );
	m_HUDDirLight.SetColor( 1.0f, 1.0f, 1.0f );

	// Save our player index so we can make sure to always reference the right player
	FASSERT(nPlayerIndex >= 0);
	m_nPlayerIdx = nPlayerIndex;

	m_bNoRadar = FALSE;

	m_pTransmissionAntennaMeshEntity = NULL;

	//////////////////////////////////////
	// Stuff from old static InitLevel
	//
	// Need to set the hud mode based on what type of bot was associated with this player.
	// The player bot is already setup by this time and in some cases the hud
	// mode has already been set, don't stomp on it. - Mike Starich
	CPlayer& rPlayer = Player_aPlayer[nPlayerIndex];
	FASSERT(Player_aPlayer[nPlayerIndex].m_pEntityCurrent && (Player_aPlayer[nPlayerIndex].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
	CBot::SetHudMode( this, ((CBot *)rPlayer.m_pEntityOrig)->m_pBotDef );

	// Set the draw enable mask. This is used as a mask with the hud draw flags
	// to allow items to be enabled and disabled without regard to hud mode.
	m_uDrawEnableFlags = 0xffffffff;
	#if (!_RADAR_ON)
		m_uDrawEnableFlags &= ~DRAW_RADAR;
	#endif

	m_bRedTint = TRUE;

	m_uButtons = 0;
	m_uButtonsLatched = 0;

	m_bDrawEnabled = TRUE;
	//	m_bDrawOnlyWashers = FALSE;
	m_bFirstFrame = TRUE;

	m_bDualWasSelected = FALSE;
	m_auFlashing[0] = 0;
	m_auFlashing[1] = 0;

	//	m_fCurThetaY = FMATH_PI * 0.5f;

	QSInit2();

	m_eSDState = SDSTATE_IDLE;

	m_nWhichWeaponSelectIsActive = -1;
	m_eWeaponSelectState = WEAPONSELECTSTATE_IDLE;

	m_bWSEnabled = TRUE;
	m_fWSBoxBPUnitPos = 0.0f;
	m_fWSBoxDir = 0.0f;
	m_fWSBoxDestBPUnitPos = 0.0f;
	m_fWSBoxCurThetaY = -FMATH_HALF_PI;

	m_nTransmissionState = TRANSMISSION_STATE_IDLE;
	m_fTransmissionsTimer = 0.0f;
	m_pTransmissionAudioEmitter = NULL;
	m_pwszTransmissionAuthor = NULL;
	m_bTransmissionAbortWithCutScene = FALSE;

	m_pTransmissionAntennaMeshEntity = fnew CMeshEntity;
	if( m_pTransmissionAntennaMeshEntity == NULL ) {
		DEVPRINTF( "CHud2::Create(): Not enough memory to create m_pTransmissionAntennaMeshEntity.\n" );
		goto _HudCreateExitWithError;
	}

	if( !m_pTransmissionAntennaMeshEntity->Create( _TRANSMISSION_ANTENNA_MESH_NAME ) ) {
		DEVPRINTF( "CHud2::Create(): Trouble creating m_pTransmissionAntennaMeshEntity.\n" );
		goto _HudCreateExitWithError;
	}

	m_pTransmissionAntennaMeshEntity->WriteMeshInstFlags( FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_NOLIGHT | FMESHINST_FLAG_NOBONES | FMESHINST_FLAG_NOCOLLIDE );
	m_pTransmissionAntennaMeshEntity->SetCollisionFlag( FALSE );
	m_pTransmissionAntennaMeshEntity->SetLineOfSightFlag( FALSE );
	m_pTransmissionAntennaMeshEntity->RemoveFromWorld();

	//CHudBatteryBarSystem::InitLevel();

	// 
	//////////////////////////
	
	// Set up the viewport for item pickup.
	m_pviewItemPickup = fviewport_Create();
	if (m_pviewItemPickup == NULL) {
		goto _HudCreateExitWithError;
	}

	// The logic here is a bit of a hack. Rather than mess with the old code
	// determining the bounce locations, we make sure that the bouncing item
	// which has been picked up is drawn in a 4x3 viewport inside of our 
	// full viewport.
//CPS 4.7.03	FViewport_t* pVp = rPlayer.m_pViewportPersp3D;
	pVp = rPlayer.m_pViewportPersp3D;			//CPS 4.7.03
	if (pVp->nWidth > pVp->nHeight) {
		u32 uWidth = (4 * pVp->nHeight) / 3;
		if (uWidth > pVp->nWidth)
			uWidth = pVp->nWidth;
		u32 uLeft = pVp->nScreenLeftX + ((pVp->nWidth - uWidth) >> 1);
		fviewport_InitPersp(m_pviewItemPickup, FMATH_PI * (1.0f / 6.0f), 0.1f, 1000.0f,
			uLeft, pVp->nScreenTopY, uWidth, pVp->nHeight);
	}
	else {
		u32 uHeight = (3 * pVp->nWidth) >> 2;
		if (uHeight > pVp->nHeight)
			uHeight = pVp->nHeight;
		u32 uTop = pVp->nScreenTopY + ((pVp->nHeight - uHeight) >> 1);
		fviewport_InitPersp(m_pviewItemPickup, FMATH_PI * (1.0f / 6.0f), 0.1f, 1000.0f,
			pVp->nScreenLeftX, uTop, pVp->nWidth, uHeight);
	}

	// Set up our orthogonal viewport. We should really just use our parent's...
	m_pviewOrtho3d = fviewport_Create();
	if (m_pviewOrtho3d == NULL)
		goto _HudCreateExitWithError;

	pVp = rPlayer.m_pViewportSafeOrtho3D;
	fviewport_InitOrtho3D(m_pviewOrtho3d, 0.1f, 1000.0f,
		pVp->nScreenLeftX, pVp->nScreenTopY, pVp->nWidth, pVp->nHeight);

	// Set up the two side perspective viewports.
	u32 uSide;
	for(uSide = 0; uSide < 2; ++uSide)
	{
		m_pviewPerspective[uSide] = fviewport_Create();
		if (m_pviewPerspective[uSide] == NULL)
			goto _HudCreateExitWithError;
		FViewport_t* pVp = rPlayer.m_pViewportPersp3D;
		f32 fOffsetFactor = -(2.0f * (f32)(uSide) - 1.0f);

		u32 uMeshLE_PC = pVp->nScreenLeftX + (u32)(((fWSInfoBoxMeshLE + fWSInfoBoxOffset * fOffsetFactor + 1.0f) * 0.5f) * (f32)(pVp->nWidth));
		u32 uMeshWidth_PC = (u32)(((fWSInfoBoxMeshRE - fWSInfoBoxMeshLE) * 0.5f) * (f32)(pVp->nWidth));
		u32 uMeshTE_PC = pVp->nScreenTopY + (u32)(((-fWSInfoBoxMeshTE + 0.75f) * (2.0f / 3.0f)) * (f32)(pVp->nHeight));
		u32 uMeshHeight_PC = (u32)(((fWSInfoBoxMeshTE - fWSInfoBoxMeshBE) * (2.0f / 3.0f)) * (f32)(pVp->nHeight));

		fviewport_InitPersp(m_pviewPerspective[uSide], FMATH_QUARTER_PI, 0.1f, 1000.0f, uMeshLE_PC, uMeshTE_PC, uMeshWidth_PC, uMeshHeight_PC);
	}


	//// Create an area for our WS info text box.
	//

	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 2.0f;
	oTextArea.ohFont = '3';
	oTextArea.bVisible = FALSE;

	// For the weapon name.
	for(uSide = 0; uSide < 2; ++uSide)
	{
		oTextArea.fUpperLeftX = CHud2_avecWSNameUL[uSide].x;
		oTextArea.fUpperLeftY = CHud2_avecWSNameUL[uSide].y;
		oTextArea.fLowerRightX = CHud2_avecWSNameLR[uSide].x;
		oTextArea.fLowerRightY = CHud2_avecWSNameLR[uSide].y;
		m_ahWSName[uSide] = ftext_Create(&oTextArea);
	}

	oTextArea.fNumberOfLines = 5.0f;
	oTextArea.ohFont = '8';

	// For the description.
	for(uSide = 0; uSide < 2; ++uSide)
	{
		oTextArea.fUpperLeftX = CHud2_avecWSDescUL[uSide].x;
		oTextArea.fUpperLeftY = CHud2_avecWSDescUL[uSide].y;
		oTextArea.fLowerRightX = CHud2_avecWSDescLR[uSide].x;
		oTextArea.fLowerRightY = CHud2_avecWSDescLR[uSide].y;
		m_ahWSDesc[uSide] = ftext_Create(&oTextArea);
	}

	oTextArea.fNumberOfLines = 1.0f;

	// For the EUK level.
	for(uSide = 0; uSide < 2; ++uSide)
	{
		oTextArea.fUpperLeftX = CHud2_avecWSEUKUL[uSide].x;
		oTextArea.fUpperLeftY = CHud2_avecWSEUKUL[uSide].y;
		oTextArea.fLowerRightX = CHud2_avecWSEUKLR[uSide].x;
		oTextArea.fLowerRightY = CHud2_avecWSEUKLR[uSide].y;
		m_ahWSEUK[uSide] = ftext_Create(&oTextArea);
	}

	// For the Ammo Alert text.
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.ohFont = '3';
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	for(uSide = 0; uSide < 2; ++uSide)
	{
		oTextArea.fUpperLeftX = CHud2_avecWSAmmoAlertUL[uSide].x;
		oTextArea.fUpperLeftY = CHud2_avecWSAmmoAlertUL[uSide].y;
		oTextArea.fLowerRightX = CHud2_avecWSAmmoAlertLR[uSide].x;
		oTextArea.fLowerRightY = CHud2_avecWSAmmoAlertLR[uSide].y;
		m_ahWSAmmoAlert[uSide] = ftext_Create(&oTextArea);
	}

	// For the Upgradeable text.
	oTextArea.fNumberOfLines = 2.0f;
	oTextArea.ohFont = '3';
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	for(uSide = 0; uSide < 2; ++uSide)
	{
		oTextArea.fUpperLeftX = CHud2_avecWSUpgradeableUL[uSide].x;
		oTextArea.fUpperLeftY = CHud2_avecWSUpgradeableUL[uSide].y;
		oTextArea.fLowerRightX = CHud2_avecWSUpgradeableLR[uSide].x;
		oTextArea.fLowerRightY = CHud2_avecWSUpgradeableLR[uSide].y;
		m_ahWSUpgradeable[uSide] = ftext_Create(&oTextArea);
	}

	oTextArea.fNumberOfLines = 3.0f;
	oTextArea.ohFont = '8';
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	oTextArea.fUpperLeftX = CHud2_vecWSNoScopeUL.x;
	oTextArea.fUpperLeftY = CHud2_vecWSNoScopeUL.y;
	oTextArea.fLowerRightX = CHud2_vecWSNoScopeLR.x;
	oTextArea.fLowerRightY = CHud2_vecWSNoScopeLR.y;
	m_hScopeUnusableTextArea = ftext_Create(&oTextArea);

	oTextArea.fNumberOfLines = 2.0f;
	oTextArea.ohFont = '3';
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
	// For the weapon type.
	for(uSide = 0; uSide < 2; ++uSide)
	{
		oTextArea.fUpperLeftX = CHud2_avecWSWeapTypeUL[uSide].x;
		oTextArea.fUpperLeftY = CHud2_avecWSWeapTypeUL[uSide].y;
		oTextArea.fLowerRightX = CHud2_avecWSWeapTypeLR[uSide].x;
		oTextArea.fLowerRightY = CHud2_avecWSWeapTypeLR[uSide].y;
		m_ahWSWeapType[uSide] = ftext_Create(&oTextArea);
	}

	// The text "Hold To Exit Bot" when possessing
	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 2.0f;
	oTextArea.ohFont = '1';
	oTextArea.fLowerRightX = 0.985f;
	fExitBotLeft = oTextArea.fLowerRightX - 0.15f;			//CPS 4.7.03
	oTextArea.fUpperLeftX = fExitBotLeft; //0.63f;
	oTextArea.fUpperLeftY = 0.01f;
	oTextArea.fLowerRightY = 0.07f;
	oTextArea.oColorBackground.OpaqueRed();
	oTextArea.fBorderThicknessX = 0.0f;
	oTextArea.fBorderThicknessY = 0.0f;
	oTextArea.oColorForeground.Set(0.709f, 0.2f, 0.2f, 1.0f);//OpaqueRed();
	oTextArea.bVisible = FALSE;
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;

	m_hSDText = ftext_Create(&oTextArea);

	oTextArea.fUpperLeftX += 0.003f;
	oTextArea.fUpperLeftY += 0.003f;
	oTextArea.fLowerRightX += 0.003f;
	oTextArea.fLowerRightY += 0.003f;
	oTextArea.oColorForeground.Black();
	m_hSDTextShadow = ftext_Create(&oTextArea);

	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.ohFont = '1';
	oTextArea.fUpperLeftX = 0.7f - CHud2_fSDCounterHalfWidth;
	oTextArea.fUpperLeftY = 0.08f * 0.75f;
	oTextArea.fLowerRightX = 0.7f + CHud2_fSDCounterHalfWidth;
	oTextArea.fLowerRightY = 0.08f * 0.75f + 2.0f * CHud2_fSDCounterHalfWidth;
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	m_ahSDCounter[0] = ftext_Create(&oTextArea);
	m_ahSDCounter[1] = ftext_Create(&oTextArea);

	// Letterbox and Col. Alloy messages
	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 2.0f;
	oTextArea.fLineSpacing = 0.008f;
	oTextArea.ohFont = '1';
	oTextArea.fUpperLeftX = 0.245f;
	oTextArea.fUpperLeftY = 0.630f;
	oTextArea.fLowerRightX = 0.70f;
	oTextArea.fLowerRightY = 0.725f;
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	oTextArea.oColorBackground.Set( 0.0f, 0.0f, 0.2f, _TRANSMISSION_BOX_ALPHA );
	oTextArea.oColorBorder.Set( 0.0f, 0.0f, 0.1f, 1.0f );
	oTextArea.fBorderThicknessX = 0.005f;
	oTextArea.fBorderThicknessY = 0.005f;
	oTextArea.bVisible = FALSE;
	m_hMessage = ftext_Create(&oTextArea);
	//
	////

	//// Create the text regions for the ammo boxes.
	//
	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 1.0f;

	// Initialize the font for a non-Mil hud.
	oTextArea.ohFont = CHud2_acAmmoFont[0];
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_RIGHT;
	oTextArea.bVisible = FALSE;

	oTextArea.fUpperLeftX = CHud2_avecCurAmmoUL[0].x;
	oTextArea.fUpperLeftY = CHud2_avecCurAmmoUL[0].y;
	oTextArea.fLowerRightX = CHud2_avecCurAmmoLR[0].x;
	oTextArea.fLowerRightY = CHud2_avecCurAmmoLR[0].y;
	m_ahCurAmmo[0] = ftext_Create(&oTextArea);

	oTextArea.fUpperLeftX = CHud2_avecCurAmmoUL[1].x;
	oTextArea.fUpperLeftY = CHud2_avecCurAmmoUL[1].y;
	oTextArea.fLowerRightX = CHud2_avecCurAmmoLR[1].x;
	oTextArea.fLowerRightY = CHud2_avecCurAmmoLR[1].y;
	m_ahCurAmmo[1] = ftext_Create(&oTextArea);

	oTextArea.ohFont = CHud2_acAmmoFont[1];
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_RIGHT;
	oTextArea.bVisible = FALSE;

	oTextArea.fUpperLeftX = CHud2_avecMaxAmmoUL[0].x;
	oTextArea.fUpperLeftY = CHud2_avecMaxAmmoUL[0].y;
	oTextArea.fLowerRightX = CHud2_avecMaxAmmoLR[0].x;
	oTextArea.fLowerRightY = CHud2_avecMaxAmmoLR[0].y;
	m_ahMaxAmmo[0] = ftext_Create(&oTextArea);

	oTextArea.fUpperLeftX = CHud2_avecMaxAmmoUL[1].x;
	oTextArea.fUpperLeftY = CHud2_avecMaxAmmoUL[1].y;
	oTextArea.fLowerRightX = CHud2_avecMaxAmmoLR[1].x;
	oTextArea.fLowerRightY = CHud2_avecMaxAmmoLR[1].y;
	m_ahMaxAmmo[1] = ftext_Create(&oTextArea);
	//
	////

	{
		// Set up the vertices for the detpack
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_pfDrawFloat		= NULL;
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_vTextPos.Set( fDetPackPosX1 + 0.023f, fDetPackPosY1 - 0.21f );

		f32 fS1 = 0.666f;
		f32 fS2 = 0.792f;
		f32 fT1 = 0.843f;
		f32 fT2 = 1.0f;

		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[0].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[0].Pos_MS.Set(fDetPackPosX1, fDetPackPosY1, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[0].ST.Set(fS1, fT1);

		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[1].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[1].Pos_MS.Set(fDetPackPosX2, fDetPackPosY1, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[1].ST.Set(fS2, fT1);

		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[2].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[2].Pos_MS.Set(fDetPackPosX1, fDetPackPosY2, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[2].ST.Set(fS1, fT2);

		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[3].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[3].Pos_MS.Set(fDetPackPosX2, fDetPackPosY2, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_DETPACK ].m_aVertices[3].ST.Set(fS2, fT2);
	}

	{
		// Set up the vertices for the race timer
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_pfDrawFloat		= NULL;
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_vTextPos.Set( fRaceTimerPosX1 + 0.023f, fRaceTimerPosY1 - 0.21f );

		f32 fS1 = 0.804f;
		f32 fS2 = 0.933f;
		f32 fT1 = 0.84f;
		f32 fT2 = 1.0f;

		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[0].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[0].Pos_MS.Set(fRaceTimerPosX1, fRaceTimerPosY1, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[0].ST.Set(fS1, fT1);

		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[1].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[1].Pos_MS.Set(fRaceTimerPosX2, fRaceTimerPosY1, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[1].ST.Set(fS2, fT1);

		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[2].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[2].Pos_MS.Set(fRaceTimerPosX1, fRaceTimerPosY2, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[2].ST.Set(fS1, fT2);

		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[3].ColorRGBA.OpaqueWhite();
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[3].Pos_MS.Set(fRaceTimerPosX2, fRaceTimerPosY2, 0.0f);
		m_aIconTimerData[ ICON_TIMER_TYPE_RACE ].m_aVertices[3].ST.Set(fS2, fT2);
	}

	// init timer vars & vertices
	//// Set up the vertices for the radar.
	fS1 = 146.0f / 256.0f;
	fS2 = 240.0f / 256.0f;//238.0f / 256.0f;
	fT1 = 146.0f / 256.0f;
	fT2 = 239.0f / 256.0f;
	
	m_avtxRadar[0].ColorRGBA.OpaqueWhite();
	m_avtxRadar[0].Pos_MS.Set(fRadarPosX, fRadarPosY, 0.0f);
	m_avtxRadar[0].ST.Set(fS1, fT1);

	m_avtxRadar[1].ColorRGBA.OpaqueWhite();
	m_avtxRadar[1].Pos_MS.Set(fRadarPosX + fRadarWidth, fRadarPosY, 0.0f);
	m_avtxRadar[1].ST.Set(fS2, fT1);

	m_avtxRadar[2].ColorRGBA.OpaqueWhite();
	m_avtxRadar[2].Pos_MS.Set(fRadarPosX, fRadarPosY - fRadarHeight, 0.0f);
	m_avtxRadar[2].ST.Set(fS1, fT2);

	m_avtxRadar[3].ColorRGBA.OpaqueWhite();
	m_avtxRadar[3].Pos_MS.Set(fRadarPosX + fRadarWidth, fRadarPosY - fRadarHeight, 0.0f);
	m_avtxRadar[3].ST.Set(fS2, fT2);
	//
	////

	//// Set up the vertices for the arrow.
	//
	fS1 = 0.0f;//160.0f / 256.0f;
	fS2 = 0.25f;//192.0f / 256.0f;
	fT1 = 0.753f;//0.0f / 256.0f;
	fT2 = 1.0f;//32.0f / 256.0f;

	// Note : Colors should be set up at the point of use.
	// 0 = Upward.
	m_aavtxArrow[0][0].Pos_MS.Set(fQSArrowLE, fQSArrowTE, 0.0f);
	m_aavtxArrow[0][0].ST.Set(fS1, fT1);

	m_aavtxArrow[0][1].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE, 0.0f);
	m_aavtxArrow[0][1].ST.Set(fS2, fT1);

	m_aavtxArrow[0][2].Pos_MS.Set(fQSArrowLE, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[0][2].ST.Set(fS1, fT2);

	m_aavtxArrow[0][3].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[0][3].ST.Set(fS2, fT2);

	// 1 = Downward.
	m_aavtxArrow[1][0].Pos_MS.Set(fQSArrowLE, fQSArrowTE, 0.0f);
	m_aavtxArrow[1][0].ST.Set(fS1, fT2);

	m_aavtxArrow[1][1].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE, 0.0f);
	m_aavtxArrow[1][1].ST.Set(fS2, fT2);

	m_aavtxArrow[1][2].Pos_MS.Set(fQSArrowLE, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[1][2].ST.Set(fS1, fT1);

	m_aavtxArrow[1][3].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[1][3].ST.Set(fS2, fT1);

	// 2 = Leftward.
	m_aavtxArrow[2][0].Pos_MS.Set(fQSArrowLE, fQSArrowTE, 0.0f);
	m_aavtxArrow[2][0].ST.Set(fS2, fT2);

	m_aavtxArrow[2][1].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE, 0.0f);
	m_aavtxArrow[2][1].ST.Set(fS2, fT1);

	m_aavtxArrow[2][2].Pos_MS.Set(fQSArrowLE, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[2][2].ST.Set(fS1, fT2);

	m_aavtxArrow[2][3].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[2][3].ST.Set(fS1, fT1);

	// 3 = Rightward.
	m_aavtxArrow[3][0].Pos_MS.Set(fQSArrowLE, fQSArrowTE, 0.0f);
	m_aavtxArrow[3][0].ST.Set(fS1, fT1);

	m_aavtxArrow[3][1].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE, 0.0f);
	m_aavtxArrow[3][1].ST.Set(fS1, fT2);

	m_aavtxArrow[3][2].Pos_MS.Set(fQSArrowLE, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[3][2].ST.Set(fS2, fT1);

	m_aavtxArrow[3][3].Pos_MS.Set(fQSArrowLE + fQSArrowWidth, fQSArrowTE - fQSArrowHeight, 0.0f);
	m_aavtxArrow[3][3].ST.Set(fS2, fT2);
	//
	////

//CPS 4.7.03	u32 uCurVtx = 0;
	uCurVtx = 0;			//CPS 4.7.03	
	f32 fX1, fX2, fY1, fY2;

	// The picture of the X button for the "Hold To Exit" message in 
	// possesion mode.
	fX2 = FTextXToJCX(fExitBotLeft) - 0.01f; //0.25f;
	fX1 = fX2 - 0.1f; //0.15f;
	fY1 = 0.72f; //0.62f;
	fY2 = 0.62f; //0.52f;

#if FANG_PLATFORM_DX
	fS1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_X].x;
	fT1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_X].y;
	fS2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_X].x;
	fT2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_X].y;
#elif FANG_PLATFORM_GC
	fS1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].x;
	fT1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].y;
	fS2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].x;
	fT2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].y;
#else
	fS1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_X].x;
	fT1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_X].y;
	fS2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_X].x;
	fT2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_X].y;
#endif

	m_avtxButton[uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
	m_avtxButton[uCurVtx].ST.Set(fS1, fT1);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
	m_avtxButton[uCurVtx].ST.Set(fS2, fT1);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
	m_avtxButton[uCurVtx].ST.Set(fS1, fT2);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
	m_avtxButton[uCurVtx].ST.Set(fS2, fT2);
	//
	//////////////////////////////////////////////////////////////////////

	u32 uWeaponSide, uCurWSIdx;
	for(uWeaponSide = 0; uWeaponSide < 2; ++uWeaponSide)
	{
		for(uCurWSIdx = 0; uCurWSIdx < 20; ++uCurWSIdx)
		{
			m_aWSItem[uWeaponSide][uCurWSIdx].Init();
		}
	}

	m_Battery.Create();
	m_oWasherDisplay.Create();

	ResetRadar();

	//////////////////////////////////////////////////////////////////////
	//
	m_eTextDisplayState = TEXTDISPLAY_STATE_IDLE;
	m_fTextDisplayTime = 0.0f;
	m_fTextFlashTime = 0.0f;
	m_fTextPulseAngle = 0.0f;
	m_pTextBuffer = NULL;
	m_TextColor.OpaqueWhite();

	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 2.0f;
	oTextArea.fLineSpacing = 0.008f;
	oTextArea.ohFont = '1';
	oTextArea.fUpperLeftX = 0.1f;
	oTextArea.fUpperLeftY = 0.630f - 0.5f;
	oTextArea.fLowerRightX = 0.90f;
	oTextArea.fLowerRightY = 0.725f - 0.5f;
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	oTextArea.oColorBackground.Set( 1.0f, 1.0f, 1.0f, 0.0f );
	oTextArea.oColorBorder.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	oTextArea.fBorderThicknessX = 0.0f;
	oTextArea.fBorderThicknessY = 0.0f;
	oTextArea.bVisible = FALSE;
	m_hTextDisplay = ftext_Create(&oTextArea);
	//
	//////////////////////////////////////////////////////////////////////

	// Done!
	return TRUE;

_HudCreateExitWithError:
	return FALSE;
}

void CHud2::Destroy(s32 nPlayerIndex)
{
	fdelete( m_pTransmissionAntennaMeshEntity );
	m_pTransmissionAntennaMeshEntity = NULL;

	m_oWasherDisplay.Destroy();
	m_Battery.Destroy();

	u32 i;
	for(i = 0; i < 3; i++)
		m_aoHudItemInst[i].Clear();

	_TextDisplayRelease();
}
// =============================================================================================================

BOOL CHud2::InitSystem()
{
	_bSystemInitialized = TRUE;

	// load the sfx bank
	fresload_Load( FSNDFX_RESTYPE, "GUI" );

	CHudBatteryBarSystem::InitSystem();

	return(TRUE);
}

// =============================================================================================================

void CHud2::UninitSystem()
{
	CHudBatteryBarSystem::Shutdown();

	_bSystemInitialized = FALSE;
}

// =============================================================================================================

BOOL CHud2::InitLevel()
{
	FASSERT(_bSystemInitialized);
	_bLevelInitialized = TRUE;

	m_fAmmoAlertUnitCountdownTimer = 0.0f;

	// Don't pause to switch weapons for multiplayer
	_bPauseToSwitchWeapons = (CPlayer::m_nPlayerCount == 1);

	//////////////////////////////////////////////////////////////////////
	// Load the sound handles.
	// Ignore errors because it will run without the sounds, and the sound
	// doesn't work under Windows.
	_hScrollClick = fsndfx_GetFxHandle("HUD Click");
	_hSelect = fsndfx_GetFxHandle("HUD Select");

	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Load shared texture defs
	CHudWSItem::m_tex.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2hudall1")) );
	if(CHudWSItem::m_tex.GetTexDef() == NULL) {
		DEVPRINTF("CHud2::InitSystem() : Could not load 'tfh2hudall1'.\n");
		goto _Hud2InitLevelErrorExit;
	}

	//// Load the info box background texture.
	//
	_texInfoBox.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh1hudall2"/*"TFH2invbg02"*/)) );
	if(_texInfoBox.GetTexDef() == NULL) {
		DEVPRINTF("CHud2::InitSystem() : Could not load info box background texture.\n");
		goto _Hud2InitLevelErrorExit;
	}

	_texHud.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2hudall1")) );
	if(_texHud.GetTexDef() == NULL) {
		DEVPRINTF("CHud2::InitSystem() : Could not load HUD 'tfh2hudall1' texture.\n");
		goto _Hud2InitLevelErrorExit;
	}
	//
	////

	//////////////////////////////////////////////////////////////////////
	// Init the button icon textures.
	_texControls.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, CPauseScreen_pszControlsTex)) );
	if(_texControls.GetTexDef() == NULL) {
		DEVPRINTF("CPauseScreen::InitSystem() : Could not load button texture.\n");
		goto _Hud2InitLevelErrorExit;
	}

	_texControls.ClearFlag( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T | CFTexInst::FLAG_WRAP_U );

	CHudBatteryBarSystem::InitLevel();

	return TRUE;

_Hud2InitLevelErrorExit:
	_bLevelInitialized = FALSE;
	return FALSE;
}

// =============================================================================================================

void CHud2::EndLevel()
{
	FASSERT(_bSystemInitialized);
	_bLevelInitialized = FALSE;

	CHudBatteryBarSystem::EndLevel();

	if (_bPauseToSwitchWeapons)
		floop_PauseGame(FALSE);	// Just in case...

	_hScrollClick = NULL;
	_hSelect = NULL;

	// TODO: How do you unload or clear:
	//	_texInfoBox
	//	_texHud
	//	_texControls
}

// =============================================================================================================

void CHud2::SetDrawEnabled(BOOL bDrawEnabled)
{
	FASSERT(_bSystemInitialized);
		
	m_eQSState = QSSTATE_WAITINGFORCLEANSTART;

	if (bDrawEnabled)
	{
		CReticle::EnableDrawAll( TRUE );
	}
	Player_aPlayer[ m_nPlayerIdx ].m_Reticle.EnableDraw( bDrawEnabled );

	m_bDrawEnabled = bDrawEnabled;
}

void CHud2::OverrideHealthBar( BOOL bOverride, f32 *pfData, u32 uNumBatteries, const CFColorRGBA &startColor, const CFColorRGBA &endColor ) {
	if( bOverride ) {
		m_bOverrideHealth			= TRUE;
		m_pfHealthOverrideData		= pfData;
		m_uHealthOverrideBatteries	= uNumBatteries;
		m_HealthOverrideStartColor	= startColor;
		m_HealthOverrideEndColor	= endColor;
	} else {
		m_bOverrideHealth = FALSE;
	}
}


BOOL CHud2::OverrideWeaponBox( BOOL bOverride, u32 uSide, cchar *pszWeaponName, BOOL bReverse/*=FALSE*/ ) {
	FASSERT( uSide < 2 );

	if( bOverride ) {
		CItem *pItem = CItemRepository::RetrieveEntry( pszWeaponName, NULL );
		if( pItem == NULL ) {
			return FALSE;
		}
		return OverrideWeaponBox( TRUE, uSide, pItem, bReverse );
	} else {
		m_abOverrideWeaponBox[uSide] = FALSE;
	}

	return TRUE;
}

BOOL CHud2::OverrideWeaponBox( BOOL bOverride, u32 uSide, CItem *pItem, BOOL bReverse/*=FALSE*/ ) {
	FASSERT( uSide < 2 );

	if( bOverride ) {
		m_abOverrideWeaponBox[uSide]		= TRUE;
		m_apOverrideWeaponBoxItem[uSide]	= pItem;
		m_abOverrideWeaponBoxReverse[uSide]	= bReverse;
	} else {
		m_apOverrideWeaponBoxItem[uSide] = FALSE;
	}

	return TRUE;
}


void CHud2::OverrideAmmoData( u32 uSide, u32 uOverrideMode, f32 *pfData, BOOL bOverrideColor, CFColorRGBA *pStartColor/*=NULL*/,
																							  CFColorRGBA *pEndColor/*=NULL*/ ) {
	FASSERT( uOverrideMode < OVERRIDE_MODE_COUNT );
	FASSERT( uSide < 2 );
	
	m_auAmmoOverrideMode[uSide]		= uOverrideMode;		
	m_apfAmmoOverrideData[uSide]	= pfData;
	m_auAmmoOverrideStyle[uSide]	= (uOverrideMode == OVERRIDE_METER_MIL) || (uOverrideMode == OVERRIDE_COUNTER_MIL);

	if( bOverrideColor ) {
		FASSERT( pStartColor != NULL );
		FASSERT( pEndColor != NULL );

		m_abAmmoOverrideColor[uSide] = TRUE;
		m_aAmmoOverrideStartColorRGB[uSide] = *pStartColor;
		m_aAmmoOverrideEndColorRGB[uSide] = *pEndColor;
	} else {
		m_abAmmoOverrideColor[uSide] = FALSE;
	}
}

BOOL CHud2::SetIconTimerDraw( IconTimerType_t nTimer, BOOL bDraw, f32* pTimer )
{
	if( nTimer < 0 || nTimer >= NUM_ICON_TIMERS )
	{
		FASSERT_NOW;
		return FALSE;
	}

	if( bDraw )
	{
		if ( pTimer && m_aIconTimerData[ nTimer ].m_pfDrawFloat && (*m_aIconTimerData[ nTimer ].m_pfDrawFloat < *pTimer) ) // ignore requests to draw over with bigger values
		{
			return TRUE;
		}

		m_uDrawFlags |= CHud2::DRAW_ICON_TIMER;
		m_aIconTimerData[ nTimer ].m_pfDrawFloat = pTimer;
		m_nCurrentIconTimer = nTimer;
	}
	else
	{
		m_uDrawFlags &= ~CHud2::DRAW_ICON_TIMER;
		m_aIconTimerData[ nTimer ].m_pfDrawFloat = NULL;
	}
	return TRUE;
}

// =============================================================================================================

BOOL CHud2::IsDrawEnabled()
{
	FASSERT(_bSystemInitialized);
	return(m_bDrawEnabled);
}

// =============================================================================================================

void CHud2::SetHudMode(HudMode_e eHudMode)
{
	if((m_eCurHudMode != HUDMODE_MIL) && (eHudMode == HUDMODE_MIL))
	{
		ftext_GetAttributes(m_ahCurAmmo[0])->fUpperLeftX += 0.026f;
		ftext_GetAttributes(m_ahCurAmmo[0])->fLowerRightX += 0.026f;
		ftext_GetAttributes(m_ahCurAmmo[1])->fUpperLeftX -= 0.026f;
		ftext_GetAttributes(m_ahCurAmmo[1])->fLowerRightX -= 0.026f;
	}
	else if((m_eCurHudMode == HUDMODE_MIL) && (eHudMode != HUDMODE_MIL))
	{
		ftext_GetAttributes(m_ahCurAmmo[0])->fUpperLeftX -= 0.026f;
		ftext_GetAttributes(m_ahCurAmmo[0])->fLowerRightX -= 0.026f;
		ftext_GetAttributes(m_ahCurAmmo[1])->fUpperLeftX += 0.026f;
		ftext_GetAttributes(m_ahCurAmmo[1])->fLowerRightX += 0.026f;
	}
	m_eCurHudMode = eHudMode;
	m_Battery.SetHudMode(eHudMode);
	switch(m_eCurHudMode)
	{
		case HUDMODE_GLITCH:
		{
			m_uDrawFlags	= DRAW_GLITCH_HUD;
			m_eSDState		= SDSTATE_IDLE;
			break;
		}
		case HUDMODE_SLOSH:
		{
			m_uDrawFlags 	= DRAW_SLOSH_HUD;
			m_eSDState		= SDSTATE_IDLE;
			break;
		}
		case HUDMODE_KRUNK:
		{
			m_uDrawFlags 	= DRAW_KRUNK_HUD;
			m_eSDState		= SDSTATE_IDLE;
			break;
		}
		case HUDMODE_MOZER:
		{
			m_uDrawFlags 	= DRAW_MOZER_HUD;
			m_eSDState		= SDSTATE_IDLE;
			break;
		}
		case HUDMODE_MIL:
		{
			m_uDrawFlags	= DRAW_POSESSED_MIL;
			m_eSDState		= SDSTATE_MESSAGE;
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================

void CHud2::SetRedTint(BOOL bRedTint)
{
	m_bRedTint = bRedTint;
}

// =============================================================================================================

f32 CHud2::GetUnitSelfDestruct()
{
	if((m_eSDState == SDSTATE_IDLE) || (m_eSDState == SDSTATE_MESSAGE))
	{
		return(0.0f);
	}

	f32 fBase = 1.0f - (f32)(m_nSDCountDown) * (1.0f / 3.0f);
	fBase += (1.0f - m_fSDTimer * CHud2_fOOSDCounterTime) * (1.0f / 3.0f);

	FMATH_CLAMP(fBase, 0.0f, 1.0f);

	return(fBase);
}

// =============================================================================================================

BOOL CHud2::TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, FSndFx_FxHandle_t hSndFx, f32 fUnitVolume, BOOL bAbortWithCutScene ) {
	TransmissionMsg_Stop( FALSE );

	if( nAuthorIndex<0 || nAuthorIndex>=TRANSMISSION_AUTHOR_COUNT ) {
		return FALSE;
	}

	if( hSndFx == FSNDFX_INVALID_FX_HANDLE ) {
		return FALSE;
	}

	m_pTransmissionAudioEmitter = FSNDFX_ALLOCNPLAY2D( hSndFx, fUnitVolume, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
	if( m_pTransmissionAudioEmitter == NULL ) {
		return FALSE;
	}

	_TransmissionMsg_Start( nAuthorIndex, bAbortWithCutScene );

	return TRUE;
}


BOOL CHud2::TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, cchar *pszStreamingFileName, f32 fUnitVolume, BOOL bAbortWithCutScene ) {
	TransmissionMsg_Stop( FALSE );

	if( nAuthorIndex<0 || nAuthorIndex>=TRANSMISSION_AUTHOR_COUNT ) {
		return FALSE;
	}

	if( pszStreamingFileName==NULL || pszStreamingFileName[0]==0 ) {
		return FALSE;
	}

	level_PlayStreamingSpeech( pszStreamingFileName, fUnitVolume );

	_TransmissionMsg_Start( nAuthorIndex, bAbortWithCutScene );

	return TRUE;
}


void CHud2::_TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, BOOL bAbortWithCutScene ) {
	static GamePhrase_e __anAuthorGamePhrase[TRANSMISSION_AUTHOR_COUNT] = {
		GAMEPHRASE_COLONEL_ALLOY,
		GAMEPHRASE_AGENT_SHHH,
		GAMEPHRASE_KRUNK,
		GAMEPHRASE_AGENT_GOFF,
		GAMEPHRASE_DR_EXAVOLT,
	};

	m_nTransmissionState = TRANSMISSION_STATE_START_ON1;
	m_fTransmissionsTimer = _TRANSMISSION_FLASH_SECS;
	m_pwszTransmissionAuthor = Game_apwszPhrases[ __anAuthorGamePhrase[nAuthorIndex] ];
	m_bTransmissionAbortWithCutScene = bAbortWithCutScene;

	CEntity *pGlitch = Player_aPlayer[m_nPlayerIdx].m_pEntityOrig;
	if( pGlitch->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
		CFMtx43A AntennaMtx;

		AntennaMtx.Identity();
		AntennaMtx.m_vPos.y = _TRANSMISSION_ANTENNA_OFFSET_Y;

		m_pTransmissionAntennaMeshEntity->AddToWorld();
		m_pTransmissionAntennaMeshEntity->Attach_UnitMtxToParent_PS_NewScale_WS(
												pGlitch,
												_TRANSMISSION_ANTENNA_ATTACH_BONE,
												&AntennaMtx,
												_TRANSMISSION_ANTENNA_SCALE,
												TRUE
											);
	}
}


void CHud2::TransmissionMsg_Stop( BOOL bFadeOut ) {
	if( m_nTransmissionState == TRANSMISSION_STATE_IDLE ) {
		return;
	}

	if( bFadeOut ) {
		// Fade out current message...

		m_nTransmissionState = TRANSMISSION_STATE_FADING_OUT;
		m_fTransmissionsTimer = _TRANSMISSION_FADEOUT_SECS;
	} else {
		// Kill current message...

		if( m_pTransmissionAudioEmitter ) {
			m_pTransmissionAudioEmitter->Destroy();
			m_pTransmissionAudioEmitter = NULL;
		} else {
			level_StopStreamingSpeech();
		}

		m_nTransmissionState = TRANSMISSION_STATE_IDLE;
		m_fTransmissionsTimer = 0.0f;
		m_pwszTransmissionAuthor = NULL;

		m_pTransmissionAntennaMeshEntity->RemoveFromWorld();
	}
}


BOOL CHud2::TransmissionMsg_IsDonePlaying( void ) const {
	return (m_nTransmissionState == TRANSMISSION_STATE_IDLE);
}


BOOL CHud2::Transmission_GetAbortWithCutSceneFlag( void ) const {
	if( m_nTransmissionState == TRANSMISSION_STATE_IDLE ) {
		return FALSE;
	}

	return m_bTransmissionAbortWithCutScene;
}


// =============================================================================================================

BOOL CHud2::StartWeaponSelect(u32 uWhichSide, CInventory *pInventory, BOOL bDelay)
{
	FASSERT(_bSystemInitialized);
	FASSERT(_bLevelInitialized);

	FASSERT(m_eWeaponSelectState <= 4);

	if( pInventory == NULL )
	{
		return(FALSE);
	}

	if(m_eQSState != QSSTATE_WAITINGFORCLEANSTART)
	{
		m_eQSState = QSSTATE_WAITINGFORCLEANSTART;
	}

	if(!m_bWSEnabled)
	{
		// They've turned off weapon select.
		return(FALSE);
	}

	if(m_eWeaponSelectState != WEAPONSELECTSTATE_IDLE)
	{
		// They're probably restarting a weapon select before it was completely off.  We handle this elsewhere.
		return(FALSE);
	}
	
	if ( (m_eCurHudMode == HUDMODE_SLOSH) || // only does reload
		 (m_eCurHudMode == HUDMODE_KRUNK) ||
		 (m_eCurHudMode == HUDMODE_MIL) ||
		 (m_eCurHudMode == HUDMODE_MOZER) )
	{
		if( pInventory->m_pfcnCallback )
		{
			pInventory->m_pfcnCallback(IREASON_RELOAD, pInventory, 0, 0);
		}
		return(FALSE);
	}

	// If we're not Glitch, or the weapon select system is not enabled, do not enable weapon selection
	if( (m_eCurHudMode != HUDMODE_GLITCH) || !DrawFlagsEnabled(DRAW_WEAPONSELECT) )
	{
		// Only Blink can go into the weapon select screen.
		return(FALSE);
	}

	if((m_nWhichWeaponSelectIsActive != -1) && (m_nWhichWeaponSelectIsActive != (s32)(uWhichSide)))
	{
		// They're trying to initiate for one side when they've already started the other.  Just ignore it.
		return(FALSE);
	}

	m_eQSState = QSSTATE_WAITINGFORCLEANSTART;

	m_nWhichWeaponSelectIsActive = uWhichSide;
	m_uWSNumVisible = FMATH_MIN(uMaxWSItemsVisible, pInventory->m_auNumWeapons[uWhichSide]);
	m_auWSCurSelected[0] = pInventory->m_auCurWeapon[0];
	m_auWSCurSelected[1] = pInventory->m_auCurWeapon[1];
	m_uWSInventorySize = pInventory->m_auNumWeapons[uWhichSide];

	//ME:  grenades & RL are ok now
	//if(pInventory->m_aoWeapons[0][m_auWSCurSelected[0]].m_pItemData->IsDual())
	//{
	//	FASSERT(m_auWSCurSelected[1] == 0);
	//	m_bDualWasSelected = TRUE;
	//}
	//else
	//{
	//	m_bDualWasSelected = FALSE;
	//}

	//////////////////////////////////////////////////////////////////////
	// Load the data from the inventory object into our WS item list.
	u32 uCurWSIdx, uCurSide;
	for(uCurSide = 0; uCurSide < 2; ++uCurSide)
	{
		for(uCurWSIdx = 0; uCurWSIdx < m_uWSInventorySize; ++uCurWSIdx)
		{
			m_aWSItem[uCurSide][uCurWSIdx].m_vecStartUL.Set(afWeaponLEOffScreen[uCurSide], afWeaponTE[uCurSide] + ((fWeaponIconHeight + fWSSpacing) * ((uCurWSIdx - m_auWSCurSelected[uCurSide] + m_uWSInventorySize) % m_uWSInventorySize)), 0.0f);
			m_aWSItem[uCurSide][uCurWSIdx].m_vecEndUL.Set(afWeaponLE[uCurSide], afWeaponTE[uCurSide] + ((fWeaponIconHeight + fWSSpacing) * ((uCurWSIdx - m_auWSCurSelected[uCurSide] + m_uWSInventorySize) % m_uWSInventorySize)), 0.0f);
			if(uCurWSIdx != m_auWSCurSelected[uCurSide])
			{
				m_aWSItem[uCurSide][uCurWSIdx].m_vecCurUL = m_aWSItem[uCurSide][uCurWSIdx].m_vecStartUL;
				m_aWSItem[uCurSide][uCurWSIdx].UpdateTextArea(uCurSide);
			}
			m_aWSItem[uCurSide][uCurWSIdx].m_pItemInst = &(pInventory->m_aoWeapons[uCurSide][uCurWSIdx]);
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	FASSERT(m_eWeaponSelectState <= 4);

	m_nWSPendingScroll = 0;
	m_fWSTimeCountdown = 0.0f;

//	m_fThetaY1 = FMATH_PI * 0.50f;
	m_fFIAlpha1 = 0.0f;
	if(bDelay)
	{
//		m_fThetaY1 *= CHud2_fDelayConstant;//CHud2_fThetaOverConstant;
		m_fFIAlpha1 += (-0.5f * CHud2_fDelayConstant);
	}
//	m_fThetaY2 = 0.0f;

	m_fFIAlpha2 = CHud2_fFadeOutAlpha;

	m_bWSScrollCleared = FALSE;

	m_eWeaponSelectState = WEAPONSELECTSTATE_SCROLLINGON;

	return(TRUE);
}

// =============================================================================================================

void CHud2::SetWasher(BOOL bVisible)
{
	if(m_eCurHudMode == HUDMODE_MIL)
		return;

	if(bVisible)
	{
		if(m_oWasherDisplay.m_eState == WASHERSTATE_OFF)
		{
			m_oWasherDisplay.m_fTimer = 0.0f;
			m_oWasherDisplay.m_eState = WASHERSTATE_SLIDINGON;
		}
	}
	else
	{
		if(m_oWasherDisplay.m_eState != WASHERSTATE_OFF)
		{
			m_oWasherDisplay.m_fTimer = 0.0f;
			m_oWasherDisplay.m_eState = WASHERSTATE_SLIDINGOFF;
		}
	}
}

// =============================================================================================================

void CHud2::SetWasherTimed(f32 fTimeOn)
{
	if(m_eCurHudMode == HUDMODE_MIL)
		return;

	m_oWasherDisplay.m_fTimeTilOff = fTimeOn;
	m_oWasherDisplay.m_fTimer = 0.0f;

	CFVec3 vecTemp(fWasherIconLE, fWasherIconTE, 0.0f);
	m_oWasherDisplay.SetUL(&vecTemp, m_pviewOrtho3d);
	m_oWasherDisplay.m_eState = WASHERSTATE_TIMEDON;
}

// =============================================================================================================

void CHud2::PickupItemGeneric(CCollectableType *pCollectableType, ItemType_e eItemType, f32 fScale)
{
	FASSERT(_bSystemInitialized);
	FASSERT(_bLevelInitialized);
	FASSERT( pCollectableType );

	s32 nIdx = -1;

	//// Find an appropriate slot for the item to go to.
	//
	u32 uCurHudItemInst;
	for(uCurHudItemInst = 0; uCurHudItemInst < 3; ++uCurHudItemInst)
	{
		if(m_aoHudItemInst[uCurHudItemInst].m_eItemState == ITEMSTATE_EMPTY)
		{
			if(nIdx == -1)
				nIdx = uCurHudItemInst;
		}
		else
		{
			if(m_aoHudItemInst[uCurHudItemInst].m_eItemType == eItemType)
			{
				nIdx = uCurHudItemInst;
				break;
			}
		}
	}
	//
	////

	//// We found a slot where the item can go.
	//
	if ((nIdx != -1) && m_aoHudItemInst[nIdx].InitFromMI(pCollectableType))
	{
		m_aoHudItemInst[nIdx].m_eItemType = eItemType;
		m_aoHudItemInst[nIdx].m_fScale = fScale;
		// REPAIR:
//		bot_PlaySound(Bot_pPlayerBot, BOT_SOUND_GET_POWERUP, &(Bot_pPlayerBot->Pos_WS), 1.0f);
	}
	//
	////
}

// =============================================================================================================

void CHud2::PickupItemBattery()
{
	m_Battery.StartBatteryPickup();
}

// =============================================================================================================

void CHud2::GlobalWork( void )
{
	m_fAmmoAlertUnitCountdownTimer -= HUD2_AMMO_ALERT_FREQ * FLoop_fRealPreviousLoopSecs;
	if( m_fAmmoAlertUnitCountdownTimer <= 0.0f )
	{
		m_fAmmoAlertUnitCountdownTimer = 1.0f;
	}
}

void CHud2::Work(CInventory *pInventory)
{
	FASSERT(_bSystemInitialized);
	FASSERT(_bLevelInitialized);
	FASSERT(m_eWeaponSelectState <= 4);
	FASSERT(Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent && (Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));

	if( pInventory == NULL )
	{
		return;
	}

	// Just to simplify the code below, get our player pointer
	CPlayer* pPlayer = &Player_aPlayer[m_nPlayerIdx];

	// For multiplayer, we do our own very simplified switching
	// DFS temp disable quickcycle	if ( MultiplayerMgr.IsSinglePlayer() || DrawFlagsEnabled(DRAW_SELFDESTRUCT) )
		CheckControls();
	// DFS temp disable quickcycle	else
	// DFS temp disable quickcycle		QuickCycleWork(pInventory);

	//// Do the work for the items that are flying up to the item pickup region.
	//
	u32 uCurHudItemInst;
	for(uCurHudItemInst = 0; uCurHudItemInst < 3; ++uCurHudItemInst)
	{
		switch(m_aoHudItemInst[uCurHudItemInst].m_eItemState)
		{
			case ITEMSTATE_EMPTY:
			{
				break;
			}
			case ITEMSTATE_JUMPINGUP:
			{
				m_aoHudItemInst[uCurHudItemInst].m_fTotalTime += FLoop_fPreviousLoopSecs;

				if(m_aoHudItemInst[uCurHudItemInst].m_fTotalTime > fItemPickupJumpTime)
				{
					m_aoHudItemInst[uCurHudItemInst].m_fTimeCounter = 0.0f;
					m_aoHudItemInst[uCurHudItemInst].m_eItemState = ITEMSTATE_SCROLLINGOFF;
				}
				else
				{
					CFVec3 vecCurPos_CS;
					f32 fUnitTime = m_aoHudItemInst[uCurHudItemInst].m_fTotalTime * fOOItemPickupJumpTime;

					f32 fWeight1, fWeight2;
					fWeight1 = 1.0f - fUnitTime;
					fWeight2 = fUnitTime * (2.0f - fUnitTime);

					vecCurPos_CS.x = fWeight1 * m_aoHudItemInst[uCurHudItemInst].m_vecStartPos_CS.x + fWeight2 * m_aoHudItemInst[uCurHudItemInst].m_vecEndPos_CS.x;
					vecCurPos_CS.y = m_aoHudItemInst[uCurHudItemInst].m_vecEndPos_CS.y - 0.75f / ((m_aoHudItemInst[uCurHudItemInst].m_fTotalTime + m_aoHudItemInst[uCurHudItemInst].m_fConstant) * (m_aoHudItemInst[uCurHudItemInst].m_fTotalTime + m_aoHudItemInst[uCurHudItemInst].m_fConstant)) + fItemPickupBounceAmplitude * fmath_Sin(fItemPickupBounceFreq * m_aoHudItemInst[uCurHudItemInst].m_fTotalTime) / (m_aoHudItemInst[uCurHudItemInst].m_fTotalTime + 0.1f);
					vecCurPos_CS.z = fWeight1 * m_aoHudItemInst[uCurHudItemInst].m_vecStartPos_CS.z + fWeight2 * m_aoHudItemInst[uCurHudItemInst].m_vecEndPos_CS.z;

					m_aoHudItemInst[uCurHudItemInst].CalcMtx(m_aoHudItemInst[uCurHudItemInst].m_fTotalTime * fItemPickupYawOmega, &vecCurPos_CS);

				}

				break;
			}
			case ITEMSTATE_SCROLLINGOFF:
			{
				m_aoHudItemInst[uCurHudItemInst].m_fTimeCounter += FLoop_fPreviousLoopSecs;
				m_aoHudItemInst[uCurHudItemInst].m_fTotalTime += FLoop_fPreviousLoopSecs;
				if(m_aoHudItemInst[uCurHudItemInst].m_fTimeCounter > fItemPickupScrollOffTime)
				{
					m_aoHudItemInst[uCurHudItemInst].Empty();
//					DEVPRINTF("SCROLLINGOFF -> EMPTY\n");
				}
				else
				{
					CFVec3 vecTemp = m_aoHudItemInst[uCurHudItemInst].m_vecEndPos_CS;
					vecTemp += CFVec3(0.0f, 3.0f, 0.0f) * (m_aoHudItemInst[uCurHudItemInst].m_fTimeCounter / fItemPickupScrollOffTime);

					m_aoHudItemInst[uCurHudItemInst].CalcMtx(m_aoHudItemInst[uCurHudItemInst].m_fTotalTime * fItemPickupYawOmega, &vecTemp);
				}

				break;
			}
			default:
			{
				FASSERT_NOW;
			}
		}
	}
	//
	//// 

	//// Handle state transitions for the weapon select interface.
	//
	switch(m_eWeaponSelectState)
	{
		case WEAPONSELECTSTATE_IDLE:
		{
			// Transitions from this state are externally generated and are handled by the StartWeaponSelect() method.
			break;
		}
		case WEAPONSELECTSTATE_SCROLLINGON:
		{
			////////////////////////////////////////////////////////////////////////
			// Check to see if they've tried to scroll at all.
			if(m_bWSScrollCleared)
			{
				if((m_uButtonsLatched & JINPUT_MOVEUP) != JINPUT_NONE)
				{
//					++m_nWSPendingScroll;
					_AddPendingScroll(1);
				}

				if((m_uButtonsLatched & JINPUT_MOVEDOWN) != JINPUT_NONE)
				{
//					--m_nWSPendingScroll;
					_AddPendingScroll(-1);
				}
			}
			//
			////////////////////////////////////////////////////////////////////////

			//// Check if the weapon select button has been released.
			//
			FASSERT(m_nWhichWeaponSelectIsActive != -1);
			if(((m_uButtons & auActivateButton[m_nWhichWeaponSelectIsActive]) == 0) && (m_nWSPendingScroll == 0))
			{
				FASSERT(m_eWeaponSelectState <= 4);
				SetDestinationsToOffScreen();

				m_fFIAlpha1 = m_fCurFIAlpha;
				m_fFIAlpha2 = 0.0f;

				FASSERT(m_eWeaponSelectState <= 4);

				m_fWSTimeCountdown = 0.0f;
				if(pInventory->SetCurWeapon(m_nWhichWeaponSelectIsActive, m_auWSCurSelected[m_nWhichWeaponSelectIsActive], FALSE, FALSE))
				{
					m_auFlashing[m_nWhichWeaponSelectIsActive] = CHud2_uNumFlashes;
					m_afFlashAlpha[m_nWhichWeaponSelectIsActive] = 1.0f;
				}
				m_auWSCurSelected[0] = pInventory->m_auCurWeapon[0];
				m_auWSCurSelected[1] = pInventory->m_auCurWeapon[1];

				WSBoxSetDest(0.0f);

				if (_bPauseToSwitchWeapons)
					floop_PauseGame(FALSE);

//				fsndfx_Play2D(_hSelect);

				m_eWeaponSelectState = WEAPONSELECTSTATE_SCROLLINGOFF;
				break;
			}
			//
			////

			//// Check to see if the animation is complete.
			//
			m_fWSTimeCountdown += FLoop_fRealPreviousLoopSecs;
			if(m_fWSTimeCountdown >= fWSScrollingOnTime)
			{
				FASSERT(m_eWeaponSelectState <= 4);
				SetCursToDests();
				FASSERT(m_eWeaponSelectState <= 4);
				m_fWSItemTheta = 0.0f;

				if (_bPauseToSwitchWeapons) {
					floop_PauseGame(TRUE);

					//pause the  sounds only, NOT THE STREAMS...
					//Also, dont pause sounds that are a pause level 2 or above
					if (m_bAudioPause) {
						CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_1 );
					}
				}

				switch(m_nWhichWeaponSelectIsActive)
				{
					case 0:
					{
						WSBoxSetDest(1.0f);
						break;
					}
					case 1:
					{
						WSBoxSetDest(-1.0f);
						break;
					}
					default:
					{
					}
				}

				m_eWeaponSelectState = WEAPONSELECTSTATE_PAUSED;
				break;
			}
			//
			////

			break;
		}
		case WEAPONSELECTSTATE_PAUSED:
		{
			m_fWSItemTheta += FLoop_fRealPreviousLoopSecs * fWSItemOmegaYaw;
			if(m_bWSScrollCleared)
			{
				if((m_uButtonsLatched & JINPUT_MOVEUP) != JINPUT_NONE)
				{
//					++m_nWSPendingScroll;
					_AddPendingScroll(1);
				}
				if((m_uButtonsLatched & JINPUT_MOVEDOWN) != JINPUT_NONE)
				{
//					--m_nWSPendingScroll;
					_AddPendingScroll(-1);
				}
			}

			if(ProcessPendingScrolls())
			{
				break;
			}

			//// Check if the weapon select button has been released.
			//
			if((m_uButtons & auActivateButton[m_nWhichWeaponSelectIsActive]) == 0)
			{
				FASSERT(m_eWeaponSelectState <= 4);
				SetDestinationsToOffScreen();
				FASSERT(m_eWeaponSelectState <= 4);

				m_fWSTimeCountdown = 0.0f;
//				m_auFlashing[m_nWhichWeaponSelectIsActive] = CHud2_uNumFlashes;
//				m_afFlashAlpha[m_nWhichWeaponSelectIsActive] = 1.0f;
				if(pInventory->SetCurWeapon(m_nWhichWeaponSelectIsActive, m_auWSCurSelected[m_nWhichWeaponSelectIsActive], TRUE, FALSE))
				{
					m_auFlashing[m_nWhichWeaponSelectIsActive] = CHud2_uNumFlashes;
					m_afFlashAlpha[m_nWhichWeaponSelectIsActive] = 1.0f;

					s32 nInactiveWS = 1 - m_nWhichWeaponSelectIsActive;
					if(m_auWSCurSelected[nInactiveWS] != pInventory->m_auCurWeapon[nInactiveWS])
					{
						// The other weapon was also forced to change.
						m_auFlashing[nInactiveWS] = CHud2_uNumFlashes;
						m_afFlashAlpha[nInactiveWS] = 1.0f;

						m_auWSCurSelected[nInactiveWS] = pInventory->m_auCurWeapon[nInactiveWS];
						m_aWSItem[nInactiveWS][m_auWSCurSelected[nInactiveWS]].m_vecCurUL.Set(afWeaponLE[nInactiveWS], afWeaponTE[nInactiveWS], 0.0f);
						m_aWSItem[nInactiveWS][m_auWSCurSelected[nInactiveWS]].UpdateTextArea(nInactiveWS);
					}
				}
				m_auWSCurSelected[0] = pInventory->m_auCurWeapon[0];
				m_auWSCurSelected[1] = pInventory->m_auCurWeapon[1];

				WSBoxSetDest(0.0f);

				m_fFIAlpha1 = m_fCurFIAlpha;
				m_fFIAlpha2 = 0.0f;

				if (_bPauseToSwitchWeapons) {
					floop_PauseGame(FALSE);

					//restart the paused sounds...
					CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_NONE );
				}

//				fsndfx_Play2D(_hSelect);

				m_eWeaponSelectState = WEAPONSELECTSTATE_SCROLLINGOFF;
				break;
			}
			//
			////

			break;
		}
		case WEAPONSELECTSTATE_SCROLLINGTO:
		{
			FASSERT(m_nWhichWeaponSelectIsActive != -1);

			if((m_uButtonsLatched & JINPUT_MOVEUP) != JINPUT_NONE)
			{
//				++m_nWSPendingScroll;
				_AddPendingScroll(1);
			}

			if((m_uButtonsLatched & JINPUT_MOVEDOWN) != JINPUT_NONE)
			{
//				--m_nWSPendingScroll;
				_AddPendingScroll(-1);
			}

			//// Check to see if the animation is complete.
			//
//			m_fWSTimeCountdown += FLoop_fPreviousLoopSecs;
			m_fWSTimeCountdown += FLoop_fRealPreviousLoopSecs;
			if(m_fWSTimeCountdown >= fWSScrollingToTime)
			{
				SetCursToDests();

//				FASSERT(m_fCurThetaY == 0.0f);
				m_eWeaponSelectState = WEAPONSELECTSTATE_PAUSED;
			}
			//
			////

			break;
		}
		case WEAPONSELECTSTATE_SCROLLINGOFF:
		{
			FASSERT(m_nWhichWeaponSelectIsActive != -1);

			//// Check to see if the player has repressed an activate button.
			//
			if((m_uButtons & auActivateButton[m_nWhichWeaponSelectIsActive]) != 0)
			{
				FASSERT(m_eWeaponSelectState <= 4);
				// TODO : Make this a call to SetDestsToOnScreen();
				u32 uCurWSIdx;
				for(uCurWSIdx = 0; uCurWSIdx < m_uWSInventorySize; ++uCurWSIdx)
				{
					m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecStartUL = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecCurUL;
					m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecEndUL.Set(afWeaponLE[m_nWhichWeaponSelectIsActive], afWeaponTE[m_nWhichWeaponSelectIsActive] + ((fWeaponIconHeight + fWSSpacing) * ((uCurWSIdx - m_auWSCurSelected[m_nWhichWeaponSelectIsActive] + m_uWSInventorySize) % m_uWSInventorySize)), 0.0f);
				}
				FASSERT(m_eWeaponSelectState <= 4);
/*
				switch(m_nWhichWeaponSelectIsActive)
				{
					case 0:
					{
						WSBoxSetDest(1.0f);
						break;
					}
					case 1:
					{
						WSBoxSetDest(-1.0f);
						break;
					}
					default:
					{
						FASSERT_NOW;
						break;
					}
				}*/

				m_fFIAlpha1 = m_fCurFIAlpha;
				m_fFIAlpha2 = CHud2_fFadeOutAlpha;

				m_fWSTimeCountdown = 0.0f;
				m_eWeaponSelectState = WEAPONSELECTSTATE_SCROLLINGON;

				break;
			}
			//
			////

			//// Check to see if the animation is complete.
			//
//			m_fWSTimeCountdown += FLoop_fPreviousLoopSecs;
			m_fWSTimeCountdown += FLoop_fRealPreviousLoopSecs;
			if(m_fWSTimeCountdown >= fWSScrollingOnTime)
			{
				FASSERT(m_eWeaponSelectState <= 4);
				SetCursToDests();
				FASSERT(m_eWeaponSelectState <= 4);
/*
				// Turn off most of the text regions
				u32 uCurWSIdx, uCurWeaponSelected = m_auWSCurSelected[m_nWhichWeaponSelectIsActive];
				{
					if(uCurWSIdx != uCurWeaponSelected)
					{
//						m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].SetAmmoTextBoxVisible(FALSE);						
					}
				}*/

				// TODO: This FALSE below isn't quite right.
//				pInventory->SetCurWeapon(m_nWhichWeaponSelectIsActive, m_auWSCurSelected[m_nWhichWeaponSelectIsActive], !m_bDualWasSelected);
/*
				pInventory->SetCurWeapon(m_nWhichWeaponSelectIsActive, m_auWSCurSelected[m_nWhichWeaponSelectIsActive], FALSE, FALSE);
				m_auWSCurSelected[0] = pInventory->m_auCurWeapon[0];
				m_auWSCurSelected[1] = pInventory->m_auCurWeapon[1];*/

				// Update 
				s32 nInactiveWS = 1 - m_nWhichWeaponSelectIsActive;
//				m_aWSItem[nInactiveWS][m_auWSCurSelected[nInactiveWS]].m_vecCurUL.Set(afWeaponLE[nInactiveWS], afWeaponTE[nInactiveWS], 0.0f);
//				m_aWSItem[nInactiveWS][m_auWSCurSelected[nInactiveWS]].UpdateTextArea(nInactiveWS);

				m_nWhichWeaponSelectIsActive = -1;
				if((m_uButtons & auActivateButton[nInactiveWS]) != 0)
				{
					m_eWeaponSelectState = WEAPONSELECTSTATE_IDLE;
					StartWeaponSelect(nInactiveWS, pInventory, FALSE);
				}
				else
				{
					m_eWeaponSelectState = WEAPONSELECTSTATE_IDLE;
				}
				break;
			}
			//
			////

			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
	//
	////

	FASSERT(m_eWeaponSelectState <= 4);

	//// Handle work for the weapon select interface.
	//
	WSBoxWork();

	switch(m_eWeaponSelectState)
	{
		case WEAPONSELECTSTATE_IDLE:
		{
			u32 uCurWeapon, uWhichWeapon;
			for(uWhichWeapon = 0; uWhichWeapon < 2; ++uWhichWeapon)
			{
				uCurWeapon = pInventory->m_auCurWeapon[uWhichWeapon];
				if(m_bFirstFrame || (m_auWSCurSelected[uWhichWeapon] != uCurWeapon))
				{
					m_aWSItem[uWhichWeapon][uCurWeapon].m_pItemInst = &(pInventory->m_aoWeapons[uWhichWeapon][uCurWeapon]);
					m_aWSItem[uWhichWeapon][uCurWeapon].m_vecCurUL.Set(afWeaponLE[uWhichWeapon], afWeaponTE[uWhichWeapon], 0.0f);
					m_aWSItem[uWhichWeapon][uCurWeapon].UpdateTextArea(uWhichWeapon);
					m_auWSCurSelected[uWhichWeapon] = uCurWeapon;
				}
			}

			break;
		}
		case WEAPONSELECTSTATE_SCROLLINGON:
		case WEAPONSELECTSTATE_SCROLLINGOFF:
		case WEAPONSELECTSTATE_SCROLLINGTO:
		{
			f32 fUnitTime;
			if(m_eWeaponSelectState == WEAPONSELECTSTATE_SCROLLINGTO)
				fUnitTime = m_fWSTimeCountdown * fOOWSScrollingToTime;
			else
				fUnitTime = m_fWSTimeCountdown * fOOWSScrollingOnTime;

			f32 fWeight1 = 1.0f - fUnitTime;
			f32 fWeight2 = fUnitTime;

			if(m_eWeaponSelectState != WEAPONSELECTSTATE_SCROLLINGTO)
			{
				m_fCurFIAlpha = fWeight1 * m_fFIAlpha1 + fWeight2 * m_fFIAlpha2;
//				FASSERT(m_fCurFIAlpha >= 0.0f);
//				FMATH_CLAMP_UNIT_FLOAT(m_fCurFIAlpha);
			}

			////
			//
			u32 uCurWSIdxOfs;
			if(m_eWeaponSelectState == WEAPONSELECTSTATE_SCROLLINGTO)
			{
				// 
				uCurWSIdxOfs = 0;
				// Do the work for the temporary block.
				DoWSItemMovement(&(m_aWSItem[m_nWhichWeaponSelectIsActive][m_uWSInventorySize]), fWeight1, fWeight2);
			}
			else
			{
				// Skip the selected block (don't scroll it off).
				uCurWSIdxOfs = 1;
			}
			//
			////

			u32 uCurWSIdx = m_auWSCurSelected[m_nWhichWeaponSelectIsActive] + uCurWSIdxOfs;
			for(; uCurWSIdxOfs < m_uWSInventorySize/*m_uWSNumVisible*/; ++uCurWSIdxOfs)
			{
				if(uCurWSIdx == m_uWSInventorySize)
					uCurWSIdx = 0;
				// Sanity check.
				FASSERT(uCurWSIdx < m_uWSInventorySize);
				DoWSItemMovement(&(m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx]), fWeight1, fWeight2);
				++uCurWSIdx;
			}
			//
			////
			
			break;
		}
		case WEAPONSELECTSTATE_PAUSED:
		{
			// TODO : Perhaps some sort of animation.
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
	//
	////

	FASSERT(m_eWeaponSelectState <= 4);

	//// Handle the state independent work for the WS flash.
	//
	u32 uSide;
	for(uSide = 0; uSide < 2; ++uSide)
	{
		if(m_auFlashing[uSide] > 0)
		{
			m_afFlashAlpha[uSide] -= FLoop_fRealPreviousLoopSecs * fOOWSFlashDuration;
			if(m_afFlashAlpha[uSide] <= 0.0f)
			{
				--m_auFlashing[uSide];
				if(m_auFlashing[uSide] > 0)
				{
					m_afFlashAlpha[uSide] = 1.0f;
				}
			}
		}
	}
	//
	////

	CEntity *pPlayerEntity;
	CBot *pPlayerBot = NULL;
	pPlayerEntity = pPlayer->m_pEntityCurrent;
	if( pPlayerEntity )
	{
		if( pPlayerEntity->TypeBits() & ENTITY_BIT_BOT )
		{
			pPlayerBot = (CBot*) pPlayerEntity;
			if( pPlayerBot->m_pDrivingVehicle && level_IsRacingLevel() && (pPlayerBot->m_pDrivingVehicle->TypeBits() & ENTITY_BIT_VEHICLERAT ) )
			{
				m_Battery.Work(pInventory, pPlayerBot->m_pDrivingVehicle->NormHealth());
			}
			else
			{
				m_Battery.Work(pInventory, pPlayer->m_pEntityCurrent->NormHealth());
			}
		}
		else
		{
			m_Battery.Work(pInventory, pPlayer->m_pEntityCurrent->NormHealth());
		}
	}

	RadarBlipWork();
	RadarWork();

	if(m_eWeaponSelectState == WEAPONSELECTSTATE_IDLE)
		QSWork2(pInventory);

	WasherWork(pInventory);

	//// Do the work for the self destruct.
	//
	// Only do this if the self DRAW_SELFDESTRUCT flag is active
	if( DrawFlagsEnabled( DRAW_SELFDESTRUCT ) ) {

		if( pPlayerBot ) {
			if( !pPlayerBot->IsPossessionExitable() ) {
				m_eSDState = SDSTATE_IDLE;	// don't show exit message if not possession exitable
			} else if( m_eCurHudMode == HUDMODE_MIL && m_eSDState == SDSTATE_IDLE ) {
				m_eSDState = SDSTATE_MESSAGE;	// restore exit message 
			}
		}

		switch(m_eSDState)
		{
			case SDSTATE_IDLE:
			{
				break;
			}
			case SDSTATE_MESSAGE:
			{
				if((m_uButtonsLatched & JINPUT_RIGHTBUTTON) != JINPUT_NONE)
				{
					m_fSDTimer = CHud2_fSDCounterTime;
					m_nSDCountDown = 3;
					m_eSDState = SDSTATE_COUNTINGDOWN;
				}
				break;
			}
			case SDSTATE_COUNTINGDOWN:
			{
				if((m_uButtons & JINPUT_RIGHTBUTTON) == JINPUT_NONE)
				{
					m_eSDState = SDSTATE_MESSAGE;
				}
				else
				{
					m_fSDTimer -= FLoop_fPreviousLoopSecs;
					if(m_fSDTimer <= 0.0f)
					{
						m_fSDTimer += CHud2_fSDCounterTime;//1.0f;
						if(m_nSDCountDown != 1)
						{
							--m_nSDCountDown;
						}
						else
						{
							m_nSDCountDown = 1;
							m_fSDTimer = 0.0f;
						}
					}
				}
				break;
			}
		}
	} else {
		// reset countdown state if drawing becomes disabled.
		// prevents getting stuck in countdown state if drawing is disabled
		// while counting down. (e.g. if countdown starts, then hud mode 
		// changes because a vehicle or site weapon was entered.)
		m_eSDState = SDSTATE_IDLE;
	}
	//
	////

	_TransmissionWork();
	_TextDisplayWork();

	m_bFirstFrame = FALSE;
}



// =============================================================================================================


// Called when the pause screen is active. Needed to disable certain text boxes from drawing.
void CHud2::NoDraw( void ) {
	if( m_nTransmissionState != TRANSMISSION_STATE_IDLE ) {
		FTextArea_t *pTextArea = ftext_GetAttributes( m_hMessage );
		pTextArea->bVisible = FALSE;
	}
}


void CHud2::Draw( CInventory *pInventory ) {
	FASSERT(_bSystemInitialized);
	FASSERT(_bLevelInitialized);

	FTextArea_t *pTextArea = ftext_GetAttributes( m_hMessage );
	pTextArea->bVisible = FALSE;

	if( !pInventory ) {
		return;
	}

	if( !DrawFlagsEnabled(DRAW_ENABLEOVERRIDE) ) {
		m_auAmmoOverrideMode[0] = m_auAmmoOverrideMode[1]	= OVERRIDE_NONE;
		m_bOverrideHealth = FALSE;
		m_abOverrideWeaponBox[0] = m_abOverrideWeaponBox[1] = FALSE;
	}

	f32 fScale, fTransX, fTransY;
	CFXfm xfmScale, xfmTotal;
	FViewport_t *pVP;

	if( m_nTransmissionState != TRANSMISSION_STATE_IDLE ) {
		if( (m_eWeaponSelectState != WEAPONSELECTSTATE_PAUSED) && (m_eWeaponSelectState != WEAPONSELECTSTATE_SCROLLINGTO) ) {
			FTextArea_t *pTextArea = ftext_GetAttributes( m_hMessage );
			pTextArea->bVisible = TRUE;
			f32 fUnitAlpha = 1.0f;

			if( (m_nTransmissionState != TRANSMISSION_STATE_START_OFF1) && (m_nTransmissionState != TRANSMISSION_STATE_START_OFF2) ) {
				pVP = Player_aPlayer[m_nPlayerIdx].m_pViewportSafeOrtho3D;
				fviewport_SetActive(pVP);

				if( CPlayer::m_nPlayerCount > 1 ) {
					fScale = 0.25f * (f32)FVid_Mode.nPixelsAcross;
				} else {
					fScale = pVP->HalfRes.x;
				}

				xfmScale.BuildScale(fScale);
				fTransY = (0.75f * fScale) - pVP->HalfRes.y;

				fTransX = pVP->HalfRes.x - fScale;
				xfmTotal.BuildTranslation(fTransX, fTransY, 1.0f);
				xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
				xfmTotal.PushModel();

				if( m_nTransmissionState == TRANSMISSION_STATE_FADING_OUT ) {
					fUnitAlpha = m_fTransmissionsTimer * (1.0f / _TRANSMISSION_FADEOUT_SECS);
				}

				Hud2_XFormPrintf( m_hMessage, L"~C707099%02u%ls\n%ls", (u32)(fUnitAlpha * 99.0f), Game_apwszPhrases[GAMEPHRASE_INCOMING_TRANSMISSION], m_pwszTransmissionAuthor );

				xfmTotal.PopModel();
			}

			pTextArea->oColorBackground.fAlpha = fUnitAlpha * _TRANSMISSION_BOX_ALPHA;
			pTextArea->oColorBorder.fAlpha = fUnitAlpha;
		}
	}

	// Text message display
	if( m_eTextDisplayState != TEXTDISPLAY_STATE_IDLE && m_fTextFlashTime >= 0.0f ) {
		if( (m_eWeaponSelectState != WEAPONSELECTSTATE_PAUSED) && (m_eWeaponSelectState != WEAPONSELECTSTATE_SCROLLINGTO) && !MultiplayerMgr.ExclusiveText( m_nPlayerIdx ) ) {
			f32 fUnitAlpha = 1.0f;

			pVP = Player_aPlayer[m_nPlayerIdx].m_pViewportSafeOrtho3D;
			fviewport_SetActive(pVP);

			// Use the same scale for each player
			if( CPlayer::m_nPlayerCount > 1 ) {
				fScale = 0.25f * (f32)FVid_Mode.nPixelsAcross;
			} else {
				fScale = pVP->HalfRes.x;
			}

			xfmScale.BuildScale(fScale);
			fTransY = pVP->HalfRes.y - (0.75f * fScale);
			fTransX = 0.0f;
			xfmTotal.BuildTranslation(fTransX, fTransY, 1.0f);
			xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
			xfmTotal.PushModel();

			if( m_eTextDisplayState == TEXTDISPLAY_STATE_FADING_OUT ) {
				fUnitAlpha = m_fTextDisplayTime * _TEXTDISPLAY_OO_FADEOUT_TIME;
			}

			Hud2_XFormPrintf( m_hTextDisplay, L"~C%02u%02u%02u%02u~S%1.2f%ls", 
				(u32) ( m_TextColor.rgba[ 0 ] * 99.0f ),
				(u32) ( m_TextColor.rgba[ 1 ] * 99.0f ),
				(u32) ( m_TextColor.rgba[ 2 ] * 99.0f ),
				(u32)(fUnitAlpha * 99.0f), 
				1.0f + ( 0.1f * FMATH_FABS( fmath_Sin( m_fTextPulseAngle ) ) ),
				m_pTextBuffer );

			xfmTotal.PopModel();
		}
	}

	if( !m_bDrawEnabled ) {
		return;
	}

	// Set up the draw renderer
	frenderer_Push(FRENDERER_DRAW, NULL);
	CFXfm::InitStack();

	fdraw_Depth_EnableWriting(FALSE);
	fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);

	// First, draw any full-viewport backdrops
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);

	// Set viewport to cover the player's entire viewport
	FViewport_t *pviewPrevious = fviewport_GetActive();
	pVP = Player_aPlayer[m_nPlayerIdx].m_pViewportOrtho3D;
	fviewport_SetActive(pVP);

	f32 fLeft   = -pVP->HalfRes.x;
	f32 fRight  = pVP->HalfRes.x;
	f32 fTop    = pVP->HalfRes.y;
	f32 fBottom = -pVP->HalfRes.y;
	f32 fZ      = 0.5f * (pVP->fFarZ + pVP->fNearZ);

	if((m_eCurHudMode == HUDMODE_MIL) && (m_bRedTint) )
	{
		FDrawVtx_t avtxTint[4];

		avtxTint[0].ColorRGBA.Red();
		avtxTint[0].ColorRGBA.fAlpha = 0.05f;
		avtxTint[0].Pos_MS.Set(fLeft, fTop, fZ);
		avtxTint[0].ST.Set(0.0f, 0.0f);

		avtxTint[1].ColorRGBA.Red();
		avtxTint[1].ColorRGBA.fAlpha = 0.05f;
		avtxTint[1].Pos_MS.Set(fRight, fTop, fZ);
		avtxTint[1].ST.Set(0.0f, 0.0f);

		avtxTint[2].ColorRGBA.Red();
		avtxTint[2].ColorRGBA.fAlpha = 0.05f;
		avtxTint[2].Pos_MS.Set(fRight, fBottom, fZ);
		avtxTint[2].ST.Set(0.0f, 0.0f);

		avtxTint[3].ColorRGBA.Red();
		avtxTint[3].ColorRGBA.fAlpha = 0.05f;
		avtxTint[3].Pos_MS.Set(fLeft, fBottom, fZ);
		avtxTint[3].ST.Set(0.0f, 0.0f);

		fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, avtxTint, 4);
	}

	if (_bPauseToSwitchWeapons) {
		// This greys out the screen while you're choosing weapons
		if( DrawFlagsEnabled(DRAW_WEAPONSELECT) ) {
			if( (m_eWeaponSelectState != WEAPONSELECTSTATE_IDLE) )
			{
				FDrawVtx_t avtxTint[4];

				CFColorRGBA rgbaTemp(0.0f, 0.0f, 0.0f, FMATH_FABS(m_fWSBoxBPUnitPos) * CHud2_fFadeOutAlpha);
				FASSERT(rgbaTemp.fAlpha >= 0.0f);

				avtxTint[0].ColorRGBA = rgbaTemp;
				avtxTint[0].Pos_MS.Set(fLeft, fTop, fZ);
				avtxTint[0].ST.Set(0.0f, 0.0f);

				avtxTint[1].ColorRGBA = rgbaTemp;
				avtxTint[1].Pos_MS.Set(fRight, fTop, fZ);
				avtxTint[1].ST.Set(0.0f, 0.0f);

				avtxTint[2].ColorRGBA = rgbaTemp;
				avtxTint[2].Pos_MS.Set(fRight, fBottom, fZ);
				avtxTint[2].ST.Set(0.0f, 0.0f);

				avtxTint[3].ColorRGBA = rgbaTemp;
				avtxTint[3].Pos_MS.Set(fLeft, fBottom, fZ);
				avtxTint[3].ST.Set(0.0f, 0.0f);

				fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, avtxTint, 4);
			}
		}
	}

	// From here we draw inside the safe area
	pVP = Player_aPlayer[m_nPlayerIdx].m_pViewportSafeOrtho3D;
	fviewport_SetActive(pVP);

	// don't draw some elements if we're selecting a weapon (at least I think that's what this is)
	BOOL bNotSelecting = (FMATH_FABS(CHud2::GetWSBoxUnitPos()) < 0.5f);

	// A note on our matrices: the rather messy code used to draw here all
	// positions elements for a 4:3 NTSC screen, where the X coordinate goes
	// from -1 (left) to 1 (right), and the y coordinate goes from 0.75 (top)
	// to -0.75 (bottom). However, our viewport is not necessarily full screen,
	// and we sometimes need to push the elements out to the corners. Also, we
	// don't necessarily want to scale in proportion to our viewport size,
	// because the elements might become too small.
	// 
	// The Z translation simply moves us past the near clip plane, because
	// our viewport system does not allow a negative near distance.

	//=========================================================================
	// Everything aligned in the upper left of the viewport
	//---------------------
	// The battery bar system. We use the full-size battery, because it is already
	// too small.
	fScale = (f32)(FVid_Mode.nPixelsAcross >> 1);

	xfmScale.BuildScale(fScale);

	fTransY = pVP->HalfRes.y - (0.75f * fScale);
	fTransX = fScale - pVP->HalfRes.x;

	xfmTotal.BuildTranslation(fTransX, fTransY, 1.0f);
	xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
	xfmTotal.PushModel();

	if( DrawFlagsEnabled(DRAW_BATTERIES) ) {
		if( m_bOverrideHealth ) {
			m_Battery.Draw( m_uHealthOverrideBatteries, m_pfHealthOverrideData, &m_HealthOverrideStartColor, &m_HealthOverrideEndColor );
		} else if( pInventory ) {
			m_Battery.Draw( pInventory->m_uNumBatteries );
		}
	}

	//---------------------
	// Icon Timer
	s32 nTimer;
	for( nTimer = 0; nTimer < NUM_ICON_TIMERS; nTimer++ )
	{
		if( m_aIconTimerData[ nTimer ].m_pfDrawFloat )
		{
			IconTimerDraw();
			break;
		}
	}

	// using HUD draw flags was problematic due to multiple parties manipulating them
	// without knowledge of current timer status.
//	if ( DrawFlagsEnabled(DRAW_ICON_TIMER) && bNotSelecting ){
//		IconTimerDraw();
//	}

	CFXfm::PopModel();

	//=========================================================================
	// Upper right corner of the viewport
	//---------------------
	// Draw the radar a bit smaller than normal if our window is small
	fScale = 0.5f * pVP->HalfRes.x + ((f32)(FVid_Mode.nPixelsAcross >> 2));
	xfmScale.BuildScale(fScale);

	fTransX = pVP->HalfRes.x - fScale;
	fTransY = pVP->HalfRes.y - (0.75f * fScale);
	xfmTotal.BuildTranslation(fTransX, fTransY, 1.0f);
	xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
	xfmTotal.PushModel();

	if( Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent && (Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) ) {
		CBot *pBot = (CBot *)Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent;

	}

	BOOL bDrawRadar = FALSE;

	if( !m_bNoRadar ) {
		if( m_eCurHudMode == HUDMODE_GLITCH ) {
			if( DrawFlagsEnabled( DRAW_RADAR ) && (m_oWasherDisplay.m_eState == WASHERSTATE_OFF) ) {
				if( Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent && (Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) ) {
					CBot *pBot = (CBot *)Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent;

					if( pBot->GetCurMech() == NULL ) {
						bDrawRadar = TRUE;
					}
				}
			}
		}
	}

	if( bDrawRadar ) {
		fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECALTEX_AT);
		fdraw_SetTexture(&_texHud);

		if( bNotSelecting ) {
			RadarDraw1();
		}
	}

	// Draw the Xbox X button for the hold-to-exit possession message
	if( DrawFlagsEnabled(DRAW_SELFDESTRUCT) ) {
		if(m_eSDState == SDSTATE_MESSAGE)
		{
#if defined(MA_PC_INPUT)
			const u32 nPromptPort = Player_aPlayer[m_nPlayerIdx].m_nControllerIndex;
			if( pcinput_UseKeyboardPromptsForPort( nPromptPort ) || pcinput_UsePlayStationPromptsForPort( nPromptPort ) ) {
				// The possession exit icon is laid out in the radar's local 640x480 space. Draw the
				// replacement in viewport space so the keyboard cap and PS glyph match its position
				// at every resolution and in split screen.
				const f32 fLocalX = 0.5f * (m_avtxButton[0].Pos_MS.x + m_avtxButton[1].Pos_MS.x);
				const f32 fLocalY = 0.5f * (m_avtxButton[0].Pos_MS.y + m_avtxButton[2].Pos_MS.y);
				const f32 fScreenX = fTransX + fScale * fLocalX;
				const f32 fScreenY = fTransY + fScale * fLocalY;
				const f32 fIconW = fScale * FMATH_FABS( m_avtxButton[1].Pos_MS.x - m_avtxButton[0].Pos_MS.x );
				const f32 fIconH = fScale * FMATH_FABS( m_avtxButton[0].Pos_MS.y - m_avtxButton[2].Pos_MS.y );
				CFXfm::PopModel();
				if( pcinput_UsePlayStationPromptsForPort( nPromptPort ) ) {
					wpr_drawutils_DrawPlayStationGlyph( 3, fScreenX - pVP->HalfRes.x, pVP->HalfRes.y - fScreenY,
						0.40f * FMATH_MIN( fIconW, fIconH ) );
				} else {
					wpr_drawutils_DrawKeyCapCentered( L"Q", fScreenX / pVP->Res.x, fScreenY / pVP->Res.y, L'C',
						fIconH / ( pVP->Res.y * 0.041f ), 0.0f, pVP->HalfRes.x, pVP->HalfRes.y, NULL, NULL, NULL, NULL );
				}
				xfmTotal.PushModel();
				fdraw_SetTexture( &_texControls );
				fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AT );
			} else
#endif
			{
				fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECALTEX_AT);
				fdraw_SetTexture(&_texControls);
				fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, m_avtxButton, 4);
			}
		}
	}

	// Draw the 2D elements of the washer display as needed (the current count)
	if( DrawFlagsEnabled(DRAW_WASHERS) ) {
		m_oWasherDisplay.Draw2D();
	}

	if ( bDrawRadar ) {
		fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);

		// DFS -- Not sure why we draw the blips when we don't draw the radar;
		// maybe a timer needs to be updated? For now I leave this as it was,
		// but it could probably be brought into the if statement with RadarDraw2.
		RadarBlipDraw();

		if( bNotSelecting )	{
			RadarDraw2();
		}
	}

	if( DrawFlagsEnabled(DRAW_SELFDESTRUCT) ) {
//		Hud2_PrintString(m_hMessage, m_wszMessage);

		switch(m_eSDState)
		{
		case SDSTATE_MESSAGE:
			{
				const u32 nPromptPort = Player_aPlayer[m_nPlayerIdx].m_nControllerIndex;
				Hud2_PrintString(m_hSDTextShadow, game_GetPromptPhrase( GAMEPHRASE_HOLD_TO_EXIT_BOT, nPromptPort ) );
				Hud2_PrintString(m_hSDText, game_GetPromptPhrase( GAMEPHRASE_HOLD_TO_EXIT_BOT, nPromptPort ) );
				break;
			}
		case SDSTATE_COUNTINGDOWN:
			{
				f32 fUnitInto = m_fSDTimer * CHud2_fOOSDCounterTime;
				f32 fDelWidth = (/*1.0f - */fUnitInto) * CHud2_fSDCounterHalfDelWidth;
				f32 fTop = CHud2_fSDCounterHalfDelWidth;
				f32 fLeft = JCXToFTextX(fRadarPosX)- CHud2_fSDCounterHalfWidth - CHud2_fSDCounterHalfDelWidth;

				ftext_GetAttributes(m_ahSDCounter[0])->fUpperLeftX = fLeft - CHud2_fSDCounterHalfWidth - fDelWidth;
				ftext_GetAttributes(m_ahSDCounter[0])->fUpperLeftY = fTop - fDelWidth;
				ftext_GetAttributes(m_ahSDCounter[0])->fLowerRightX = fLeft + CHud2_fSDCounterHalfWidth + fDelWidth;
				ftext_GetAttributes(m_ahSDCounter[0])->fLowerRightY = fTop + 2.0f * CHud2_fSDCounterHalfWidth + fDelWidth;

				//			f32 fSqrtUnitInto = fmath_Sqrt(fUnitInto);
				ftext_GetAttributes(m_ahSDCounter[0])->oColorForeground.Set(1.0f, fUnitInto * fUnitInto, fUnitInto * fUnitInto, 1.0f);
				//			ftext_GetAttributes(m_ahSDCounter[0])->oColorForeground.Set(1.0f, fSqrtUnitInto, fSqrtUnitInto, 1.0f);
				ftext_GetAttributes(m_ahSDCounter[0])->fAlpha = fUnitInto;

				Hud2_XFormPrintf(m_ahSDCounter[0], "%d", m_nSDCountDown);
				break;
			}
		}
	}

	CFXfm::PopModel();

	//=========================================================================
	// Bottom of viewport. Scale, drawing function, and Y translation don't 
	// change between left and right.
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);

	// Use the same scale for each player
	if (CPlayer::m_nPlayerCount > 1)
		fScale = 0.25f * (f32)FVid_Mode.nPixelsAcross;
	else
		fScale = pVP->HalfRes.x;
		
	//fScale = 0.75f * pVP->HalfRes.x + ((f32)(FVid_Mode.nPixelsAcross >> 3));
	xfmScale.BuildScale(fScale);
	fTransY = (0.75f * fScale) - pVP->HalfRes.y;

	//=========================================================================
	// Bottom-right corner of viewport
	//---------------------
	// Draw all the weapon select boxes
	fTransX = pVP->HalfRes.x - fScale;	// To line up right
	xfmTotal.BuildTranslation(fTransX, fTransY, 1.0f);
	xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
	xfmTotal.PushModel();

	// Draw all the 2D stuff for weapon selection and the currently selected
	// weapon icons in the lower right corner, including text.
	if( DrawFlagsEnabled(DRAW_WEAPONSYSTEM) ) {
		WSItemDraw(0);
	}

	// Draw the boxes on the bottom of the screen showing how much ammo or
	// weapon power is left, including text.
	if( DrawFlagsEnabled(DRAW_AMMOBOXES) ) {
		DrawAmmoBox(0, pInventory);
	}

	// If necessary, draw the white flash on the weapon select box
	if( DrawFlagsEnabled(DRAW_WEAPONSELECT) ) {
		WSFlashDraw(0);
	}

	// Quick-select arrows
	if( DrawFlagsEnabled(DRAW_QUICKSELECT) ) {
		fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
		QSDraw2();
	}

	CFXfm::PopModel();

	//=========================================================================
	// Bottom left corner
	//---------------------
	// Weapon select boxes
	fTransX = fScale - pVP->HalfRes.x;	// To line up left
	xfmTotal.BuildTranslation(fTransX, fTransY, 1.0f);
	xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
	xfmTotal.PushModel();

	// Draw all the 2D stuff for weapon selection and the currently selected
	// weapon icons in the lower corners, including text.
	if( DrawFlagsEnabled(DRAW_WEAPONSYSTEM) ) {
		WSItemDraw(1);
	}

	// Draws the boxes on the bottom of the screen showing how much ammo or
	// weapon power is left, including text.
	if( DrawFlagsEnabled(DRAW_AMMOBOXES) ) {
		DrawAmmoBox(1, pInventory);
	}

	// If necessary, draw the white flash on the weapon select box
	if( DrawFlagsEnabled(DRAW_WEAPONSELECT) ) {
		WSFlashDraw(1);
	}

	CFXfm::PopModel();

	//=========================================================================
	// Center-top items, with no X translation

	// DFS -- nothing is currently drawn center top, but leave this commented
	// code here, so if something center top is desired, it can be uncommented.

	//////fScale = pVP->HalfRes.x;
	//////xfmScale.BuildScale(fScale);
	//////fTransY = pVP->HalfRes.y - (0.75f * fScale);
	//////xfmTotal.BuildTranslation(0.0f, fTransY, 1.0f);
	//////xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
	//////xfmTotal.PushModel();

	//// Insert drawing code here

	//////CFXfm::PopModel();


	frenderer_Pop();

	//=========================================================================
	// Mesh items below here
	//=========================================================================

	frenderer_Push(FRENDERER_MESH, NULL);

	// Draw a bouncing 3D picture of an item we have picked up (power-up, goody)
	// for a brief period of time.
	if( DrawFlagsEnabled(DRAW_ITEMS) ) {
		if((m_eWeaponSelectState == WEAPONSELECTSTATE_IDLE || GetWSBoxUnitPos() == 0.0f) && (CHudItemInst::m_nActiveCount > 0) )
		{
			fviewport_SetActive(m_pviewItemPickup);
			fviewport_Clear(FVIEWPORT_CLEARFLAG_DEPTH | FVIEWPORT_CLEARFLAG_STENCIL, 0.0f, 0.0f, 0.0f, 1.0f, 0);

			CFXfm::InitStack();

			fmesh_Ambient_Set(1.0f, 1.0f, 1.0f, 0.20f);

			s32 nCurHIIIdx;
			for(nCurHIIIdx = 0; nCurHIIIdx < 3; ++nCurHIIIdx)
			{
				m_aoHudItemInst[nCurHIIIdx].Draw();
			}
		}
	}

	if( DrawFlagsEnabled(DRAW_BATTERY_MESHES) ) {
		m_Battery.DrawMeshes(pInventory);
	}

	if( DrawFlagsEnabled(DRAW_WEAPONSELECT) ) {
		if(m_nWhichWeaponSelectIsActive != -1)
		{
			fviewport_SetActive(m_pviewPerspective[m_nWhichWeaponSelectIsActive]);
			fviewport_Clear(FVIEWPORT_CLEARFLAG_DEPTH | FVIEWPORT_CLEARFLAG_STENCIL, 0.0f, 0.0f, 0.0f, 1.0f, 0);

			CFXfm::InitStack();

			switch(m_eWeaponSelectState)
			{
				case WEAPONSELECTSTATE_IDLE:
				case WEAPONSELECTSTATE_SCROLLINGON:
				case WEAPONSELECTSTATE_SCROLLINGOFF:
				{
					break;
				}
				case WEAPONSELECTSTATE_PAUSED:
				case WEAPONSELECTSTATE_SCROLLINGTO:
				{
					if (_bPauseToSwitchWeapons) {
						// draw the large 3d icon in the center of the screen
						WSBoxDrawMesh(&(m_aWSItem[m_nWhichWeaponSelectIsActive][m_auWSCurSelected[m_nWhichWeaponSelectIsActive]]));
					}
					break;
				}
				default:
				{
					FASSERT_NOW;
				}
			}
		}
	}

	// Draw the 3D elements of the washer display (the rotating washer)
	if( DrawFlagsEnabled(DRAW_WASHERS) ) {
		m_oWasherDisplay.Draw3D();
	}

	frenderer_Pop();		//FRENDERER_MESH

	fviewport_SetActive(pviewPrevious);
}

// =============================================================================================================

void CHud2::SetDestinationsToOffScreen()
{
	// This assert probably doesn't need to be here, although it shouldn't hurt, either.
	FASSERT(m_eWeaponSelectState != WEAPONSELECTSTATE_SCROLLINGTO);

	FASSERT(m_nWhichWeaponSelectIsActive != -1);
	u32 uCurSelected = m_auWSCurSelected[m_nWhichWeaponSelectIsActive];

	u32 uCurWSIdx;
	for(uCurWSIdx = 0; uCurWSIdx < m_uWSInventorySize; ++uCurWSIdx)
	{
		if(uCurWSIdx != uCurSelected)
		{
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecStartUL = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecCurUL;
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecEndUL.Set(afWeaponLEOffScreen[m_nWhichWeaponSelectIsActive], afWeaponTE[m_nWhichWeaponSelectIsActive] + ((fWeaponIconHeight + fWSSpacing) * ((uCurWSIdx - m_auWSCurSelected[m_nWhichWeaponSelectIsActive] + m_uWSInventorySize) % m_uWSInventorySize)), 0.0f);
		}
	}
}

// =============================================================================================================

void CHud2::SetCursToDests()
{
	FASSERT(m_nWhichWeaponSelectIsActive != -1);
	u32 uCurWSIdx;
	for(uCurWSIdx = 0; uCurWSIdx < m_uWSInventorySize; ++uCurWSIdx)
	{
		m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecCurUL = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecEndUL;
		m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].UpdateTextArea(m_nWhichWeaponSelectIsActive);
	}
}

// =============================================================================================================

void CHud2::DoWSItemMovement(CHudWSItem *pItem, f32 fWeight1, f32 fWeight2)
{
	CFVec3 vecTemp;

	pItem->m_vecCurUL = pItem->m_vecStartUL;
	pItem->m_vecCurUL *= fWeight1;

	vecTemp = pItem->m_vecEndUL;
	vecTemp *= fWeight2;

	pItem->m_vecCurUL += vecTemp;

	FASSERT(m_nWhichWeaponSelectIsActive != -1);
	pItem->UpdateTextArea(m_nWhichWeaponSelectIsActive);
}

// =============================================================================================================

u32 CHud2::GetDPadDirN()
{
	u32 uRetVal = 0;
	s32 nPlayer = Player_aPlayer[m_nPlayerIdx].m_nControllerIndex;

	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState > 0.0f) {
		uRetVal |= 1;
	}

	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState < 0.0f) {
		uRetVal |= 2;
	}

	if( Player_aPlayer[m_nPlayerIdx].GetFourWayQuickSelect() ) {
		if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState > 0.0f) {
			uRetVal |= 4;
		}

		if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState < 0.0f) {
			uRetVal |= 8;
		}
	}

	return(uRetVal);
}

// =============================================================================================================

s32 CHud2::GetSingleDir(u32 uInput)
{
	switch(uInput)
	{
		case 0:
		{
			return(-1);
		}
		case 1:
		{
			return(0);
		}
		case 2:
		{
			return(1);
		}
		case 4:
		{
			return(2);
		}
		case 8:
		{
			return(3);
		}
		default:
		{
			return(-1);
		}
	}
}

// =============================================================================================================

void CHud2::LoadProfile(u32 uFirstChance, CInventory *pInventory)
{
	if((uFirstChance != -1 ) && (m_aQSProfile[uFirstChance].m_bIsValid))
	{
		m_auFlashing[0] = CHud2_uNumFlashes;
		m_afFlashAlpha[0] = 1.0f;
		m_auFlashing[1] = CHud2_uNumFlashes;
		m_afFlashAlpha[1] = 1.0f;
	
		// NKM - New profile loading that uses the weapon type rather than index.
		CItemInst *pPrimary = NULL;
		CItemInst *pSecondary = NULL;
		s32 i;

		// See if we have the saved primary weapon in our inventory
		for( i = 0; i < pInventory->m_auNumWeapons[ INV_INDEX_PRIMARY ]; ++i ) {
			if( pInventory->m_aoWeapons[ INV_INDEX_PRIMARY ][ i ].m_pWeapon && 
				pInventory->m_aoWeapons[ INV_INDEX_PRIMARY ][ i ].m_pWeapon->Type() == m_aQSProfile[uFirstChance].m_auSelectedWeapon[ INV_INDEX_PRIMARY ] ) {
				pPrimary = &pInventory->m_aoWeapons[ INV_INDEX_PRIMARY ][ i ];
				break;
			}
		}

		// See if we have the saved secondary weapon in our inventory
		for( i = 0; i < pInventory->m_auNumWeapons[ INV_INDEX_SECONDARY ]; ++i ) {
			if( pInventory->m_aoWeapons[ INV_INDEX_SECONDARY ][ i ].m_pWeapon ) {
				if( pInventory->m_aoWeapons[ INV_INDEX_SECONDARY ][ i ].m_pWeapon->Type() == m_aQSProfile[uFirstChance].m_auSelectedWeapon[ INV_INDEX_SECONDARY ] ) {
					pSecondary = &pInventory->m_aoWeapons[ INV_INDEX_SECONDARY ][ i ];
					break;
				}
			} else {
				// This is for the case of the empty secondary, it isn't a weapon like the empty primary, but has an item inst
				if( m_aQSProfile[uFirstChance].m_auSelectedWeapon[ INV_INDEX_SECONDARY ] == ( CWeapon::WEAPON_TYPE_COUNT + 1 ) ) {
					pSecondary = &pInventory->m_aoWeapons[ INV_INDEX_SECONDARY ][ i ];
					break;
				}
			}
		}

		// If we have it, switch to it
		if( m_aQSProfile[uFirstChance].m_auSelectedWeapon[ INV_INDEX_PRIMARY ] != CWeapon::WEAPON_TYPE_COUNT ) {
			if( pPrimary && ( pPrimary != &pInventory->m_aoWeapons[ INV_INDEX_PRIMARY ][ pInventory->m_auCurWeapon[ INV_INDEX_PRIMARY ] ] ) ) {
				pInventory->SetCurWeapon( INV_INDEX_PRIMARY, pPrimary, FALSE, TRUE );
			}
		}

		if( m_aQSProfile[uFirstChance].m_auSelectedWeapon[ INV_INDEX_SECONDARY ] != CWeapon::WEAPON_TYPE_COUNT ) {
			if( pSecondary && ( pSecondary != &pInventory->m_aoWeapons[ INV_INDEX_SECONDARY ][ pInventory->m_auCurWeapon[ INV_INDEX_SECONDARY ] ] ) ) {
				pInventory->SetCurWeapon( INV_INDEX_SECONDARY, pSecondary, FALSE, TRUE );
			}
		}
		// NKM - This method was bad since it was storing the index of the currently selected weapon.
		//		 If we restore and we don't have that weapon any longer, the index will go out of the array and will
		//		 hit garbage data.
/*
		if(pInventory->m_auCurWeapon[0] != m_aQSProfile[uFirstChance].m_auSelectedWeapon[0])
		{
			pInventory->SetCurWeapon(0, m_aQSProfile[uFirstChance].m_auSelectedWeapon[0], FALSE, TRUE);
//			pInventory->m_auCurWeapon[0] = m_aQSProfile[uFirstChance].m_auSelectedWeapon[0];
		}
		if(pInventory->m_auCurWeapon[1] != m_aQSProfile[uFirstChance].m_auSelectedWeapon[1])
		{
			pInventory->SetCurWeapon(1, m_aQSProfile[uFirstChance].m_auSelectedWeapon[1], FALSE, TRUE);
//			pInventory->m_auCurWeapon[1] = m_aQSProfile[uFirstChance].m_auSelectedWeapon[1];
		}
*/
	}
}

// =============================================================================================================

void CHud2::SaveProfile(u32 uFirstChance, CInventory *pInventory)
{
	FASSERT( pInventory );

	if(uFirstChance != -1)
	{
		// NKM - Store the type of the weapon that we have so when we go to restore the profile we can search the
		//		 inventory for the weapon type and switch to it if we have it.
		CWeapon::WeaponType_e ePrimary = CWeapon::WEAPON_TYPE_COUNT;
		CWeapon::WeaponType_e eSecondary = CWeapon::WEAPON_TYPE_COUNT;

		if( pInventory->m_aoWeapons[ INV_INDEX_PRIMARY ][ m_auWSCurSelected[ INV_INDEX_PRIMARY ] ].m_pWeapon ) {
			ePrimary = pInventory->m_aoWeapons[ INV_INDEX_PRIMARY ][ m_auWSCurSelected[ INV_INDEX_PRIMARY ] ].m_pWeapon->Type();
		}

		if( pInventory->m_aoWeapons[ INV_INDEX_SECONDARY ][ m_auWSCurSelected[ INV_INDEX_SECONDARY ] ].m_pWeapon ) {
			eSecondary = pInventory->m_aoWeapons[ INV_INDEX_SECONDARY ][ m_auWSCurSelected[ INV_INDEX_SECONDARY ] ].m_pWeapon->Type();
		} else {
			// This is for the case of the empty secondary, it isn't a weapon like the empty primary, but has an item inst
			eSecondary = (CWeapon::WeaponType_e) ( CWeapon::WEAPON_TYPE_COUNT + 1 );
		}

		m_aQSProfile[uFirstChance].SetProfile(ePrimary, eSecondary);

		// NKM - This is wrong.
		//m_aQSProfile[uFirstChance].SetProfile(m_auWSCurSelected[0], m_auWSCurSelected[1]);

		return;
	}
}

// =============================================================================================================

void CHud2::CheckControls()
{
	u32 uLastButtons = m_uButtons;

	s32 nPlayer = Player_aPlayer[m_nPlayerIdx].m_nControllerIndex;
	m_uButtons = 0;

	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState > 0.0f)
	{
		m_uButtons |= JINPUT_UP;
		if((Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) != 0)
		{
			m_uButtons |= JINPUT_UPR;
		}
	}
	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState < 0.0f)
	{
		m_uButtons |= JINPUT_DOWN;
		if((Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) != 0)
		{
			m_uButtons |= JINPUT_DOWNR;
		}
	}
	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState > 0.0f)
		m_uButtons |= JINPUT_RIGHT;
	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState < 0.0f)
		m_uButtons |= JINPUT_LEFT;

	#if FANG_PLATFORM_WIN
		if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState > 0.0f)
		{
			if((Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_FORWARD_BACKWARD]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) != 0)
			{
				m_uButtons |= JINPUT_LANALOGUPR;
			}
		}
		if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState < 0.0f)
		{
			if((Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_FORWARD_BACKWARD]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) != 0)
			{
				m_uButtons |= JINPUT_LANALOGDOWNR;
			}
		}
	#endif

	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState > 0.0f)
		m_uButtons |= JINPUT_LANALOGRIGHT;
	if(Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState < 0.0f)
		m_uButtons |= JINPUT_LANALOGLEFT;
	if((Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= JINPUT_WSLEFT;
	if((Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= JINPUT_WSRIGHT;

	if((Gamepad_aapSample[nPlayer][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= JINPUT_RIGHTBUTTON;

	if(m_eWeaponSelectState != WEAPONSELECTSTATE_IDLE)
	{
		GamepadMap_e eLastMapping = gamepad_GetMapping(nPlayer);
		gamepad_SetMapping(nPlayer, GAMEPAD_MAP_MENU);
		if((Gamepad_aapSample[nPlayer][GAMEPAD_MENU_RIGHT_SHOULDER]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) != 0)
		{
			m_uButtons |= JINPUT_RBUTTONR;
		}
		if((Gamepad_aapSample[nPlayer][GAMEPAD_MENU_LEFT_SHOULDER]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) != 0)
		{
			m_uButtons |= JINPUT_LBUTTONR;
		}
		if( (Gamepad_aapSample[nPlayer][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState == 0.0f) &&
			(Gamepad_aapSample[nPlayer][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState == 0.0f) )
		{
			m_bWSScrollCleared = TRUE;
		}
		gamepad_SetMapping(nPlayer, eLastMapping);
	}

	m_uButtonsLatched = m_uButtons & (~uLastButtons);
}

// =============================================================================================================
void CHud2::AddREToVtxList(FDrawVtx_t *avtxList, u32 &uCurVtx, f32 fSize, CHudRadarEntity *pRE)
{
	CEntity *pEntity = Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent;
	FASSERT(pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOT));
	CBot *pPlayerBot = (CBot *)(pEntity);

	pRE->CalcBSPos(pPlayerBot);
	CFVec3A vecREPos_BS = pRE->m_vecPos_BS;

	f32 fAlpha = 1.0f - 0.99f * pRE->m_fStationaryTime * fOORadarFadeOutTime;
	
	CFColorRGB rgbColor;

	if( pRE->m_bFriend ) {
		rgbColor.Set( _RADAR_BLIP_COLOR_FRIENDLY_R, _RADAR_BLIP_COLOR_FRIENDLY_G, _RADAR_BLIP_COLOR_FRIENDLY_B );
	} else {
		rgbColor.Set( _RADAR_BLIP_COLOR_ENEMY_R, _RADAR_BLIP_COLOR_ENEMY_G, _RADAR_BLIP_COLOR_ENEMY_B );
	}

	FASSERT_UNIT_FLOAT(fAlpha);

	avtxList[uCurVtx].ColorRGBA = rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(vecREPos_BS.x - fSize, vecREPos_BS.z + fSize, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(vecREPos_BS.x + fSize, vecREPos_BS.z + fSize, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(vecREPos_BS.x - fSize, vecREPos_BS.z - fSize, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(vecREPos_BS.x + fSize, vecREPos_BS.z + fSize, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(vecREPos_BS.x - fSize, vecREPos_BS.z - fSize, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbColor;
	avtxList[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(vecREPos_BS.x + fSize, vecREPos_BS.z - fSize, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	++uCurVtx;
}

// =============================================================================================================

void CHud2::DrawAmmoBox(u32 uSide, CInventory *pInventory)
{
	if( !pInventory ) {
		 return;
	}

	// Bail if drawing is not enabled for this side
	if ((uSide == 0) && !DrawFlagsEnabled(DRAW_RIGHT_AMMOBOX))
		return;
	else if ((uSide == 1) && !DrawFlagsEnabled(DRAW_LEFT_AMMOBOX))
		return;

	CItemInst *pItemInst = &pInventory->m_aoWeapons[uSide][m_auWSCurSelected[uSide]];
	if( m_auAmmoOverrideMode[uSide] == OVERRIDE_NONE &&
		(pItemInst->m_aeDisplayType[0] == CItemInst::AMMODISPLAY_NONE) && 
		(pItemInst->m_aeDisplayType[1] == CItemInst::AMMODISPLAY_NONE) )
	{
		return;
	}

	u32 uHudIdx = (m_eCurHudMode == HUDMODE_MIL) ? 1 : 0;

	FDrawVtx_t avtxAmmoBoxes[16];

	f32 fX1, fX2, fY1, fY2;
	f32 fS1, fS2, fT1, fT2;

	if( m_auAmmoOverrideMode[uSide] != OVERRIDE_NONE ) {
		uHudIdx = m_auAmmoOverrideStyle[uSide];
	}

	fX1 = CHud2_avecAmmoBoxUL[uSide].x;
	fX2 = CHud2_avecAmmoBoxLR[uSide].x;
	fY1 = CHud2_avecAmmoBoxUL[uSide].y;
	fY2 = CHud2_avecAmmoBoxLR[uSide].y;

	fS1 = CHud2_avecAmmoBoxST1[uHudIdx][uSide].x;
	fS2 = CHud2_avecAmmoBoxST2[uHudIdx][uSide].x;
	fT1 = CHud2_avecAmmoBoxST1[uHudIdx][uSide].y;
	fT2 = CHud2_avecAmmoBoxST2[uHudIdx][uSide].y;

	u32 uCurVtx = 0;
	avtxAmmoBoxes[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
	avtxAmmoBoxes[uCurVtx].ColorRGBA.OpaqueWhite();
	avtxAmmoBoxes[uCurVtx].ST.Set(fS1, fT1);
	uCurVtx++;

	avtxAmmoBoxes[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
	avtxAmmoBoxes[uCurVtx].ColorRGBA.OpaqueWhite();
	avtxAmmoBoxes[uCurVtx].ST.Set(fS2, fT1);
	uCurVtx++;

	avtxAmmoBoxes[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
	avtxAmmoBoxes[uCurVtx].ColorRGBA.OpaqueWhite();
	avtxAmmoBoxes[uCurVtx].ST.Set(fS2, fT2);
	uCurVtx++;

	avtxAmmoBoxes[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
	avtxAmmoBoxes[uCurVtx].ColorRGBA.OpaqueWhite();
	avtxAmmoBoxes[uCurVtx].ST.Set(fS1, fT2);
	uCurVtx++;

	_PrepareAmmoDisplay( pInventory, uSide, avtxAmmoBoxes, &uCurVtx );

	fdraw_SetTexture(&_texInfoBox);
	fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, avtxAmmoBoxes, uCurVtx);
}

// =============================================================================================================

BOOL CHud2::ProcessPendingScrolls()
{
	// DFS -- When using the shoulder buttons, it feels better if you reverse
	// up/down for the left/right weapon select.
	s32 nScrollDirection = 1;
	////////if (m_nWhichWeaponSelectIsActive == 1)
	////////	nScrollDirection = -1;

	//// Process any pending buttons presses.
	//
	if( (nScrollDirection * m_nWSPendingScroll) > 0)			// Pieces are scrolling down.
	{
		FASSERT(m_eWeaponSelectState <= 4);
		// Make our temp piece be in the same position as our old selected piece.
		m_aWSItem[m_nWhichWeaponSelectIsActive][m_uWSInventorySize].m_pItemInst = m_aWSItem[m_nWhichWeaponSelectIsActive][m_auWSCurSelected[m_nWhichWeaponSelectIsActive]].m_pItemInst;
		m_aWSItem[m_nWhichWeaponSelectIsActive][m_uWSInventorySize].m_vecCurUL = m_aWSItem[m_nWhichWeaponSelectIsActive][m_auWSCurSelected[m_nWhichWeaponSelectIsActive]].m_vecCurUL;

		// Snap the old selected item into the top (possibly offscreen) position.
		m_aWSItem[m_nWhichWeaponSelectIsActive][m_auWSCurSelected[m_nWhichWeaponSelectIsActive]].m_vecCurUL.y = afWeaponTE[m_nWhichWeaponSelectIsActive] + (fWeaponIconHeight + fWSSpacing) * m_uWSInventorySize;

		m_nWSPendingScroll -= nScrollDirection;
		m_auWSCurSelected[m_nWhichWeaponSelectIsActive] = (m_auWSCurSelected[m_nWhichWeaponSelectIsActive] + 1) % m_uWSInventorySize;

		//// Set new target positions for everyone.
		//
		u32 uCurWSIdx;
		for(uCurWSIdx = 0; uCurWSIdx <= m_uWSInventorySize; ++uCurWSIdx)
		{
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecStartUL = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecCurUL;
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecEndUL = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecStartUL;
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecEndUL.y -= (fWeaponIconHeight + fWSSpacing);
		}
		//
		////
		FASSERT(m_eWeaponSelectState <= 4);

		m_fWSTimeCountdown = 0.0f;
		m_eWeaponSelectState = WEAPONSELECTSTATE_SCROLLINGTO;

		fsndfx_Play2D(_hScrollClick);

		return(TRUE);
	}
	else if( (nScrollDirection * m_nWSPendingScroll) < 0 )			// Pieces are scrolling up.
	{
		FASSERT(m_eWeaponSelectState <= 4);
		m_nWSPendingScroll += nScrollDirection;
		m_auWSCurSelected[m_nWhichWeaponSelectIsActive] = (m_auWSCurSelected[m_nWhichWeaponSelectIsActive] + m_uWSInventorySize - 1) % m_uWSInventorySize;

		// Make our temp piece be in the same position as the old selected piece.
		m_aWSItem[m_nWhichWeaponSelectIsActive][m_uWSInventorySize].m_pItemInst = m_aWSItem[m_nWhichWeaponSelectIsActive][m_auWSCurSelected[m_nWhichWeaponSelectIsActive]].m_pItemInst;
		m_aWSItem[m_nWhichWeaponSelectIsActive][m_uWSInventorySize].m_vecCurUL = m_aWSItem[m_nWhichWeaponSelectIsActive][m_auWSCurSelected[m_nWhichWeaponSelectIsActive]].m_vecCurUL;

		// Snap the new selected item into the bottom position.
		m_aWSItem[m_nWhichWeaponSelectIsActive][m_auWSCurSelected[m_nWhichWeaponSelectIsActive]].m_vecCurUL.y = afWeaponTE[m_nWhichWeaponSelectIsActive] - fWeaponIconHeight - fWSSpacing;

		//// Set new target positions for everyone.
		//
		u32 uCurWSIdx;
		for(uCurWSIdx = 0; uCurWSIdx <= m_uWSInventorySize; ++uCurWSIdx)
		{
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecStartUL = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecCurUL;
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecEndUL = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecStartUL;
			m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx].m_vecEndUL.y += (fWeaponIconHeight + fWSSpacing);
		}
		//
		////
		FASSERT(m_eWeaponSelectState <= 4);

		m_fWSTimeCountdown = 0.0f;
		m_eWeaponSelectState = WEAPONSELECTSTATE_SCROLLINGTO;

		fsndfx_Play2D(_hScrollClick);

		return(TRUE);
	}
	//
	////

	return(FALSE);
}

// =============================================================================================================

void CHud2::_AddPendingScroll(s32 nDelPendingScroll)
{
	if(m_uWSInventorySize != 1)
	{
		m_nWSPendingScroll += nDelPendingScroll;
	}
}

// =============================================================================================================

void CHud2::DrawWSInfoBoxMesh(CHudWSItem *pItem)
{
	FASSERT(m_nWhichWeaponSelectIsActive != -1);

	CItemInst *pII = pItem->m_pItemInst;
	FASSERT(pII != NULL);
	CItem *pI = (CItem *)(pII->m_pItemData);
	FASSERT(pI != NULL);

	pI->DrawMesh(m_fWSItemTheta);
}

// =============================================================================================================

void CHud2::_TransmissionWork( void ) {
	switch( m_nTransmissionState ) {
	case TRANSMISSION_STATE_IDLE:
		break;

	case TRANSMISSION_STATE_START_ON1:
	case TRANSMISSION_STATE_START_ON2:
		m_fTransmissionsTimer -= FLoop_fPreviousLoopSecs;

		if( m_fTransmissionsTimer <= 0.0f ) {
			m_nTransmissionState = (m_nTransmissionState == TRANSMISSION_STATE_START_ON1) ? TRANSMISSION_STATE_START_OFF1 : TRANSMISSION_STATE_START_OFF2;
			m_fTransmissionsTimer = _TRANSMISSION_FLASH_SECS;
		}

		break;

	case TRANSMISSION_STATE_START_OFF1:
	case TRANSMISSION_STATE_START_OFF2:
		m_fTransmissionsTimer -= FLoop_fPreviousLoopSecs;

		if( m_fTransmissionsTimer <= 0.0f ) {
			if( m_nTransmissionState == TRANSMISSION_STATE_START_OFF1 ) {
				m_nTransmissionState = TRANSMISSION_STATE_START_ON2;
				m_fTransmissionsTimer = _TRANSMISSION_FLASH_SECS;
			} else {
				m_nTransmissionState = TRANSMISSION_STATE_ON;
				m_fTransmissionsTimer = 0.0f;
			}
		}

		break;

	case TRANSMISSION_STATE_ON:
		if( m_pTransmissionAudioEmitter ) {
			// Banked audio...

			if( m_pTransmissionAudioEmitter->GetState() != FAUDIO_EMITTER_STATE_PLAYING ) {
				// Fade out...
				TransmissionMsg_Stop();
			}
		} else {
			// Streaming audio...

			if( !level_IsStreamingSpeechPlaying() ) {
				// Fade out...
				TransmissionMsg_Stop();
			}
		}

		break;

	case TRANSMISSION_STATE_FADING_OUT:
		m_fTransmissionsTimer -= FLoop_fPreviousLoopSecs;

		if( m_fTransmissionsTimer <= 0.0f ) {
			// Kill transmission...

			TransmissionMsg_Stop( FALSE );
		}

		break;
	};
}


void CHud2::ResetRadar( void ) {
	u32 i;

	for( i=0; i<uMaxRadarEntities; ++i ) {
		m_aRadarEntity[i].m_bIsActive = FALSE;
		m_aoRadarBlipPool[i].m_bIsActive = FALSE;
	}
}


// =============================================================================================================
void CHud2::RadarWork()
{
	//// Pretend like all of the guys we saw before have been stationary for the last frame (we will adjust this later).
	//
	u32 uREIdx;
	for(uREIdx = 0; uREIdx < uMaxRadarEntities; ++uREIdx) {
		CHudRadarEntity *pCurRE = &(m_aRadarEntity[uREIdx]);

		if(pCurRE->m_bIsActive) {
//			if( !pCurRE->m_bFriend ) {
				pCurRE->m_fStationaryTime += FLoop_fPreviousLoopSecs;
//			}
		}
	}
	//
	////

	CFSphere sphTemp;
	CEntity *pEntity = Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent;
	FASSERT(pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOT));
	CBot *pPlayerBot = (CBot *)(pEntity);
	sphTemp.m_Pos = pPlayerBot->MtxToWorld()->m_vPos.v3;
	sphTemp.m_fRadius = fRadarRange;

	_pCurrentHud = this;
	fworld_FindTrackersIntersectingSphere( &sphTemp, FWORLD_TRACKERTYPE_MESH, RadarCallback, 0, 0, NULL, MESHTYPES_ENTITY, ENTITY_BIT_BOT ); 
/*
	CFWorldUser UserTracker;
	UserTracker.MoveTracker(sphTemp);


	// Store a ptr to myself for the radar callback
	_pCurrentHud = this;
	UserTracker.FindIntersectingTrackers(RadarCallback, FWORLD_TRACKERTYPE_MESH);
	UserTracker.RemoveFromWorld();
*/
	//// Run through the list of all the guys, and if any have been stationary for too long, get rid of them.
	//
	for(uREIdx = 0; uREIdx < uMaxRadarEntities; ++uREIdx) {
		CHudRadarEntity *pCurRE = &(m_aRadarEntity[uREIdx]);

		if((pCurRE->m_bIsActive) && (pCurRE->m_fStationaryTime >= fRadarFadeOutTime)) {
			pCurRE->m_bIsActive = FALSE;
		}
	}
	//
	////
}

// =============================================================================================================

BOOL CHud2::RadarCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	return _pCurrentHud ? _pCurrentHud->DoRadarCallback(pTracker, pVolume) : FALSE;
}

// ============================================================================
BOOL CHud2::DoRadarCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
/*
	CFWorldMesh *pTargetToCheck = (CFWorldMesh *)pTracker;
	if(pTargetToCheck->m_nUser != MESHTYPES_ENTITY)
	{
		return(TRUE);
	}

	CEntity *pEntity = (CEntity *)(pTargetToCheck->m_pUser);
	if(!(pEntity->TypeBits() & ENTITY_BIT_BOT))
	{
		return(TRUE);
	}
*/
	CBot *pBot = (CBot *)((CFWorldMesh *)pTracker)->m_pUser;
	if(pBot->NormHealth() <= 0.0f)
	{
		return TRUE;
	}

	if( !pBot->Radar_CanShowUp() ) {
		return TRUE;
	}

	if( pBot->IsDeadOrDying() || !pBot->IsInWorld() || pBot->IsMarkedForWorldRemove() ) {
		return TRUE;
	}

	// Don't show the user
	if( (pBot->m_nPossessionPlayerIndex == m_nPlayerIdx) )
	{
		return TRUE;
	}

	FASSERT(Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT);
	CBot *pPlayerBot = (CBot *)(Player_aPlayer[m_nPlayerIdx].m_pEntityCurrent);

	f32 fDistToBot2 = pBot->MtxToWorld()->m_vPos.DistSq(pPlayerBot->MtxToWorld()->m_vPos);
	if ( fDistToBot2 > (fRadarRange * fRadarRange) )
	{
		return TRUE;
	}

/*	
	CFVec3A StartPoint_WS, EndPoint_WS;

	StartPoint_WS = *(pPlayerBot->GetTagPoint(0));		 //Eye position for bots, sphere origin for entities
	EndPoint_WS.Set(pTargetToCheck->GetBoundingSphere().m_Pos);
	pPlayerBot->AppendTrackerSkipList();	//Some trackers to ignore in the LOS test.
	pBot->AppendTrackerSkipList();		//Some trackers to ignore in the LOS test.
	if(fworld_IsLineOfSightObstructed( &StartPoint_WS, &EndPoint_WS))
	{
		// Obstruction...
		return(TRUE);
	}

*/

	// Okay, it's a valid bot, so let's see if it is on the list already.
	u32 uCurREIdx;
	s32 nFreeREIdx = -1;
	BOOL bIsMoving = (pBot->m_fSpeedXZ_WS != 0.0f) || (pBot->m_nPossessionPlayerIndex ==-1 && (((FLoop_nTotalLoopTicks*FLoop_fSecsPerTick) - pBot->GetLastFireTime()) < 0.1f)) ;

	for(uCurREIdx = 0; uCurREIdx < uMaxRadarEntities; ++uCurREIdx) {
		if(m_aRadarEntity[uCurREIdx].m_bIsActive) {
			if(m_aRadarEntity[uCurREIdx].m_pWM == pBot->m_pWorldMesh) {
				if( pBot->Recruit_IsRecruited() ) {
					m_aRadarEntity[uCurREIdx].m_bFriend = aiutils_IsFriendly( pPlayerBot, pBot );
				}

				if( m_aRadarEntity[uCurREIdx].m_bFriend || bIsMoving ) {
					// It's on the list.  Let's update the entry.
					m_aRadarEntity[uCurREIdx].m_fStationaryTime = 0.0f;
					m_aRadarEntity[uCurREIdx].m_vecPos_WS = pBot->MtxToWorld()->m_vPos;
				}

				break;
			}
		} else if (nFreeREIdx==-1) {
			//hand out the lowest free slot first
			nFreeREIdx = uCurREIdx;
		}
	}

	if(uCurREIdx == uMaxRadarEntities && (nFreeREIdx != -1))
	{
		// We didn't find it on the list and there is free space, let's add it in.

		m_aRadarEntity[nFreeREIdx].m_bFriend = aiutils_IsFriendly( pPlayerBot, pBot );

		if( m_aRadarEntity[nFreeREIdx].m_bFriend || bIsMoving ) {
			m_aRadarEntity[nFreeREIdx].m_bIsActive = TRUE;
			m_aRadarEntity[nFreeREIdx].m_fStationaryTime = 0.0f;
			m_aRadarEntity[nFreeREIdx].m_pWM = pBot->m_pWorldMesh;
			m_aRadarEntity[nFreeREIdx].m_vecPos_WS	= pBot->MtxToWorld()->m_vPos;

			// TODO: This shouldn't be getting called twice.
			m_aRadarEntity[nFreeREIdx].CalcBSPos(pPlayerBot);
			CFVec3 vecTemp(m_aRadarEntity[nFreeREIdx].m_vecPos_BS.x, m_aRadarEntity[nFreeREIdx].m_vecPos_BS.z, 0.0f);

			CFColorRGB rgbColor;

			if( m_aRadarEntity[nFreeREIdx].m_bFriend ) {
				rgbColor.Set( _RADAR_BLIP_COLOR_FRIENDLY_R, _RADAR_BLIP_COLOR_FRIENDLY_G, _RADAR_BLIP_COLOR_FRIENDLY_B );
			} else {
				rgbColor.Set( _RADAR_BLIP_COLOR_ENEMY_R, _RADAR_BLIP_COLOR_ENEMY_G, _RADAR_BLIP_COLOR_ENEMY_B );
			}

			RadarBlipAdd(&vecTemp, &rgbColor, 0.2f, 0.1f, 0.02f);
		}
	}

	return TRUE;
}

// =============================================================================================================

void CHud2::RadarBlipAdd(CFVec3 *vecPos, CFColorRGB *oColor, f32 fLifetime, f32 fSpeed, f32 fInitSize)
{
	u32 uCurRBIdx;
	for(uCurRBIdx = 0; uCurRBIdx < uMaxRadarEntities; ++uCurRBIdx)
	{
		if(!m_aoRadarBlipPool[uCurRBIdx].IsActive())
			break;
	}

	if(uCurRBIdx == uMaxRadarEntities)
	{
		DEVPRINTF("CHud2::RadarBlipAdd : No room to add another radar blip.\n");
		return;
	}

	m_aoRadarBlipPool[uCurRBIdx].Set(vecPos, oColor, fLifetime, fSpeed, fInitSize);
}

// =============================================================================================================

void CHud2::RadarBlipWork()
{
	u32 uCurRBIdx;
	for(uCurRBIdx = 0; uCurRBIdx < uMaxRadarEntities; ++uCurRBIdx)
		m_aoRadarBlipPool[uCurRBIdx].Work();
}

// =============================================================================================================

void CHud2::RadarBlipDraw()
{
	FDrawVtx_t *paVtxPool = fvtxpool_GetArray( 8 * uMaxRadarEntities );
	if( !paVtxPool ) {
		return;
	}

	u32 uCurRBIdx, uCurVtx = 0;
	CHudRadarBlip *pRB;

	for(uCurRBIdx = 0; uCurRBIdx < uMaxRadarEntities; ++uCurRBIdx)
	{
		pRB = &(m_aoRadarBlipPool[uCurRBIdx]);
		if(pRB->IsActive())
		{
			pRB->AddRBToVtxList( paVtxPool, uCurVtx );
		}
	}

	fdraw_PrimList(FDRAW_PRIMTYPE_LINELIST, paVtxPool, uCurVtx);

	fvtxpool_ReturnArray( paVtxPool );
}

// =============================================================================================================

// This draws the Icon Timer background.
void CHud2::IconTimerDraw()
{
	char szTime[64];

	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECALTEX_AT);
	fdraw_SetTexture(&_texInfoBox);
	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, m_aIconTimerData[ m_nCurrentIconTimer ].m_aVertices, 4);

	if( !m_aIconTimerData[ m_nCurrentIconTimer ].m_pfDrawFloat )
	{
		return;
	}

	switch( m_nCurrentIconTimer )
	{
		case ICON_TIMER_TYPE_DETPACK:
			if( *m_aIconTimerData[ m_nCurrentIconTimer ].m_pfDrawFloat < 10.0f )
			{
				sprintf( szTime, "~f1~AL~S0.85~c991515750%.2f", *m_aIconTimerData[ m_nCurrentIconTimer ].m_pfDrawFloat );
			}
			else
			{
				sprintf( szTime, "~f1~AL~S0.85~c99151575%.2f", *m_aIconTimerData[ m_nCurrentIconTimer ].m_pfDrawFloat );
			}
			break;

		case ICON_TIMER_TYPE_RACE:
			{
				s32 nMinutes;
				s32 nSeconds;

				nMinutes = (s32) ( *m_aIconTimerData[ m_nCurrentIconTimer ].m_pfDrawFloat / 60.0f );
				nSeconds = (s32) ( *m_aIconTimerData[ m_nCurrentIconTimer ].m_pfDrawFloat - ((f32)nMinutes * 60.0f) );

				if( nMinutes < 10 )
				{
					if( nSeconds < 10 )
					{
						sprintf( szTime, "~f1~AL~S0.85~c991515750%d:0%d", nMinutes, nSeconds );
					}
					else
					{
						sprintf( szTime, "~f1~AL~S0.85~c991515750%d:%d", nMinutes, nSeconds );
					}
				}
				else
				{
					if( nSeconds < 10 )
					{
						sprintf( szTime, "~f1~AL~S0.85~c99151575%d:0%d", nMinutes, nSeconds );
					}
					else
					{
						sprintf( szTime, "~f1~AL~S0.85~c99151575%d:%d", nMinutes, nSeconds );
					}
				}
			}
			break;

		default:
			FASSERT_NOW;
			break;
	}

	_XFormPrintfLocal( m_aIconTimerData[ m_nCurrentIconTimer ].m_vTextPos.x, m_aIconTimerData[ m_nCurrentIconTimer ].m_vTextPos.y, szTime );
}

// =============================================================================================================


// =============================================================================================================
// This draws the radar background.
void CHud2::RadarDraw1()
{
	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, m_avtxRadar, 4);
}

// =============================================================================================================

// This draw the radar entities.
void CHud2::RadarDraw2()
{
	FDrawVtx_t *paVtxPool = fvtxpool_GetArray( 6 * uMaxRadarEntities );
	if( !paVtxPool ) {
		return;
	}
	u32 uCurREIdx, uCurVtx = 0;
	CHudRadarEntity *pRE;
	for(uCurREIdx = 0; uCurREIdx < uMaxRadarEntities; ++uCurREIdx)
	{
		pRE = &(m_aRadarEntity[uCurREIdx]);
		if(pRE->m_bIsActive)
		{
			AddREToVtxList( paVtxPool, uCurVtx, 0.01f, pRE );
		}
	}
	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, paVtxPool, uCurVtx);

	fvtxpool_ReturnArray( paVtxPool );
}

// =============================================================================================================



// =============================================================================================================

void CHud2::WSItemDraw(u32 uSide)
{
	FDrawVtx_t avtxWSList[6 * (uMaxWSItemsVisible + 1)];

	//// Handle the drawing for the weapon select interface.
	//
	if( _bPauseToSwitchWeapons && DrawFlagsEnabled(DRAW_WEAPONSELECT) && (uSide == m_nWhichWeaponSelectIsActive)) {
		CHudWSItem *pWSOppositeItem = &(m_aWSItem[!uSide][m_auWSCurSelected[!uSide]]);
        WSBoxDrawFDraw( &m_aWSItem[uSide][m_auWSCurSelected[uSide]], pWSOppositeItem );
	}

	switch(m_eWeaponSelectState)
	{
		case WEAPONSELECTSTATE_IDLE:
		{
			if( ((uSide == 0) && DrawFlagsEnabled(DRAW_RIGHT_WEAPONBOX)) ||
				((uSide == 1) && DrawFlagsEnabled(DRAW_LEFT_WEAPONBOX)) ) {

				CHudWSItem *pWSItem, *pWSOppositeItem;
				if( m_abOverrideWeaponBox[uSide] ) {
					pWSItem = &(m_aWSItem[uSide][0]);
					pWSItem->m_vecCurUL.Set(afWeaponLE[uSide], afWeaponTE[uSide], 0.0f);
					
					pWSItem->Draw(TRUE, uSide, FALSE, TRUE, m_apOverrideWeaponBoxItem[uSide], m_abOverrideWeaponBoxReverse[uSide] );
				} else {
					pWSItem = &(m_aWSItem[uSide][m_auWSCurSelected[uSide]]);
					pWSOppositeItem = &(m_aWSItem[!uSide][m_auWSCurSelected[!uSide]]);

					pWSItem->Draw(TRUE, uSide, FALSE, FALSE, NULL, FALSE, pWSOppositeItem);
				}
			}

			break;
		}
		case WEAPONSELECTSTATE_SCROLLINGON:
		case WEAPONSELECTSTATE_SCROLLINGOFF:
		case WEAPONSELECTSTATE_PAUSED:
		case WEAPONSELECTSTATE_SCROLLINGTO:
		{
			if( !DrawFlagsEnabled(DRAW_WEAPONSELECT) ) {
				break;
			}

			if( uSide == m_nWhichWeaponSelectIsActive ) {
				CHudWSItem *pWSItem;

				u32 uCurWSIdx_A, uCurWSIdx_S, uCurSelected;
				uCurSelected = m_auWSCurSelected[m_nWhichWeaponSelectIsActive];
				for(uCurWSIdx_S = 0; uCurWSIdx_S < m_uWSNumVisible; ++uCurWSIdx_S)
				{
					// TODO : Get rid of this %.
					uCurWSIdx_A = (uCurWSIdx_S + uCurSelected) % m_uWSInventorySize;
					pWSItem = &(m_aWSItem[m_nWhichWeaponSelectIsActive][uCurWSIdx_A]);
					CHudWSItem *pWSOppositeItem = &(m_aWSItem[!m_nWhichWeaponSelectIsActive][m_auWSCurSelected[!m_nWhichWeaponSelectIsActive]]);

					pWSItem->Draw(uCurWSIdx_A == uCurSelected, m_nWhichWeaponSelectIsActive, (uCurWSIdx_A == uCurSelected) && (m_eWeaponSelectState == WEAPONSELECTSTATE_PAUSED), FALSE, NULL, FALSE, pWSOppositeItem);
				}

				//// Draw one extra item if we are scrolling.
				//
				if(m_eWeaponSelectState == WEAPONSELECTSTATE_SCROLLINGTO)
				{
					pWSItem = &(m_aWSItem[m_nWhichWeaponSelectIsActive][m_uWSInventorySize]);
					CHudWSItem *pWSOppositeItem = &(m_aWSItem[!m_nWhichWeaponSelectIsActive][m_auWSCurSelected[!m_nWhichWeaponSelectIsActive]]);
					pWSItem->Draw(FALSE, m_nWhichWeaponSelectIsActive, FALSE, FALSE, NULL, FALSE, pWSOppositeItem);
				}
				//
				////
			}
			else {
				//// Do the drawing for the inactive weapon select.
				//
				CHudWSItem *pWSItem = &(m_aWSItem[uSide][m_auWSCurSelected[uSide]]);
				CHudWSItem *pWSOppositeItem = &(m_aWSItem[!uSide][m_auWSCurSelected[!uSide]]);

				pWSItem->Draw(TRUE, uSide, FALSE, FALSE, NULL, FALSE, pWSOppositeItem);
				//
				////
			}

			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
	//
	////
}

// =============================================================================================================

void CHud2::WSFlashDraw(u32 uSide)
{
	if(m_auFlashing[uSide] > 0) {
		u32 uCurVtx = 0;
		FDrawVtx_t avtxFlash[12];

		f32 fS[2] = { 0.0f, 0.25f };
		f32 fT1 = 0.5f;
		f32 fT2 = 0.75f;

		f32 fS1 = fS[uSide];
		f32 fS2 = fS[1 - uSide];

		avtxFlash[uCurVtx].ColorRGBA.White();
		avtxFlash[uCurVtx].ColorRGBA.fAlpha = m_afFlashAlpha[uSide];
		avtxFlash[uCurVtx].Pos_MS.Set(afWeaponLE[uSide], afWeaponTE[uSide], 0.0f);
		avtxFlash[uCurVtx].ST.Set(fS1, fT1);
		uCurVtx++;

		avtxFlash[uCurVtx].ColorRGBA.White();
		avtxFlash[uCurVtx].ColorRGBA.fAlpha = m_afFlashAlpha[uSide];
		avtxFlash[uCurVtx].Pos_MS.Set(afWeaponLE[uSide] + fWeaponIconWidth, afWeaponTE[uSide], 0.0f);
		avtxFlash[uCurVtx].ST.Set(fS2, fT1);
		uCurVtx++;

		avtxFlash[uCurVtx].ColorRGBA.White();
		avtxFlash[uCurVtx].ColorRGBA.fAlpha = m_afFlashAlpha[uSide];
		avtxFlash[uCurVtx].Pos_MS.Set(afWeaponLE[uSide] + fWeaponIconWidth, afWeaponTE[uSide] - fWeaponIconHeight, 0.0f);
		avtxFlash[uCurVtx].ST.Set(fS2, fT2);
		uCurVtx++;

		avtxFlash[uCurVtx].ColorRGBA.White();
		avtxFlash[uCurVtx].ColorRGBA.fAlpha = m_afFlashAlpha[uSide];
		avtxFlash[uCurVtx].Pos_MS.Set(afWeaponLE[uSide], afWeaponTE[uSide] - fWeaponIconHeight, 0.0f);
		avtxFlash[uCurVtx].ST.Set(fS1, fT2);
		uCurVtx++;

		fdraw_SetTexture(&_texHud);
		fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, avtxFlash, uCurVtx);
	}
}

// =============================================================================================================

void CHud2::WasherWork(CInventory *pInventory)
{
	switch(m_oWasherDisplay.m_eState)
	{
		case WASHERSTATE_OFF:
		{
			break;
		}
		case WASHERSTATE_SLIDINGON:
		{
			m_oWasherDisplay.m_fTimer += FLoop_fPreviousLoopSecs;
			if(m_oWasherDisplay.m_fTimer >= fWasherSlideTime)
			{
				CFVec3 vecTemp(fWasherIconLE, fWasherIconTE, 0.0f);
				m_oWasherDisplay.SetUL(&vecTemp, m_pviewOrtho3d);

				m_oWasherDisplay.m_eState = WASHERSTATE_ON;
			}
			break;
		}
		case WASHERSTATE_ON:
		{
			break;
		}
		case WASHERSTATE_CLINKING:
		{
			m_oWasherDisplay.m_eState = WASHERSTATE_TIMEDON;
			break;
		}
		case WASHERSTATE_TIMEDON:
		{
			m_oWasherDisplay.m_fTimeTilOff -= FLoop_fPreviousLoopSecs;
			if(m_oWasherDisplay.m_fTimeTilOff <= 0.0f)
			{
				m_oWasherDisplay.m_fTimer = 0.0f;
				m_oWasherDisplay.m_eState = WASHERSTATE_SLIDINGOFF;
			}
			break;
		}
		case WASHERSTATE_SLIDINGOFF:
		{
			m_oWasherDisplay.m_fTimer += FLoop_fPreviousLoopSecs;
			if(m_oWasherDisplay.m_fTimer >= fWasherSlideTime)
				m_oWasherDisplay.m_eState = WASHERSTATE_OFF;
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}

	switch(m_oWasherDisplay.m_eState)
	{
		case WASHERSTATE_OFF:
		{
			break;
		}
		case WASHERSTATE_SLIDINGON:
		{
			f32 fUnitTime = m_oWasherDisplay.m_fTimer * fOOWasherSlideTime;

			f32 fWeight1 = 1.0f - fUnitTime;
			f32 fWeight2 = fUnitTime;

			CFVec3 vecPos, vecTemp;
			vecPos.Set(1.0f - fWasherWidth, fWasherIconTE, 0.0f);
			vecPos *= fWeight1;

			vecTemp.Set(fWasherIconLE, fWasherIconTE, 0.0f);
			vecTemp *= fWeight2;

			vecPos += vecTemp;
			m_oWasherDisplay.SetUL(&vecPos, m_pviewOrtho3d);
			break;
		}
		case WASHERSTATE_ON:
		{
			break;
		}
		case WASHERSTATE_TIMEDON:
		{
			break;
		}
		case WASHERSTATE_CLINKING:
		{
			break;
		}
		case WASHERSTATE_SLIDINGOFF:
		{
			f32 fUnitTime = m_oWasherDisplay.m_fTimer * fOOWasherSlideTime;

			f32 fWeight1 = 1.0f - fUnitTime;
			f32 fWeight2 = fUnitTime;

			CFVec3 vecPos, vecTemp;
			vecPos.Set(fWasherIconLE, fWasherIconTE, 0.0f);
			vecPos *= fWeight1;

			vecTemp.Set(1.0f - fWasherWidth, fWasherIconTE, 0.0f);
			vecTemp *= fWeight2;

			vecPos += vecTemp;
			m_oWasherDisplay.SetUL(&vecPos, m_pviewOrtho3d);
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}

	switch(m_oWasherDisplay.m_eState)
	{
		case WASHERSTATE_SLIDINGON:
		case WASHERSTATE_ON:
		case WASHERSTATE_SLIDINGOFF:
		case WASHERSTATE_TIMEDON:
		{
			m_oWasherDisplay.m_fThetaY += FLoop_fPreviousLoopSecs * CHud2_fWasherOmegaY;
			break;
		}
	}

	if( m_oWasherDisplay.m_eState == WASHERSTATE_OFF ) {
		m_oWasherDisplay.m_nWasherCountToDraw = pInventory->m_aoItems[0].m_nClipAmmo;
		m_oWasherDisplay.m_fWasherCount = (f32)m_oWasherDisplay.m_nWasherCountToDraw;
	} else {
		// animate the display count to the actual count
		if( m_oWasherDisplay.m_nWasherCountToDraw != (u32)pInventory->m_aoItems[0].m_nClipAmmo ) {

			f32 fDiff = (f32)pInventory->m_aoItems[0].m_nClipAmmo - (f32)m_oWasherDisplay.m_nWasherCountToDraw;
			f32 fMaxDecPerSec = 45.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMP( fDiff, -fMaxDecPerSec, fMaxDecPerSec );
			m_oWasherDisplay.m_fWasherCount += fDiff;
			u32 nOldDrawCount = m_oWasherDisplay.m_nWasherCountToDraw;
			m_oWasherDisplay.m_nWasherCountToDraw = (u32)m_oWasherDisplay.m_fWasherCount;
		}
	}
}

// =============================================================================================================

void CHud2::QSInit2()
{
	m_nLastDir = -1;
	m_nDrawDir = -1;
	m_uLastDState = 0;
	m_fQSTimer = 0.0f;
	m_eQSState = QSSTATE_WAITINGFORCLEANSTART;
}

// =============================================================================================================

void CHud2::QSWork2(CInventory *pInventory)
{
	if(!m_bWSEnabled || (m_eWeaponSelectState != WEAPONSELECTSTATE_IDLE))
		return;

	if(m_eCurHudMode == HUDMODE_MIL)
	{
		m_eQSState = QSSTATE_WAITINGFORCLEANSTART;
		return;
	}

	u32 uCurDState = GetDPadDirN();
	s32 nCurDir = GetSingleDir(uCurDState);

	switch(m_eQSState)
	{
		case QSSTATE_WAITINGFORCLEANSTART:
		{
			if(uCurDState == 0)
			{
				// No direction is pressed...
				if(m_nLastDir != -1)
				{
					// There was a valid direction down before...
//					DEVPRINTF("LoadA [%d].\n", m_nLastDir);
					LoadProfile(m_nLastDir, pInventory);
					m_fQSTimer += FLoop_fPreviousLoopSecs;
					m_nDrawDir = m_nLastDir;
					m_eQSState = QSSTATE_FINISHINGARROWDISPLAY;
				}
			}
			else if((nCurDir != -1) && (uCurDState == m_uLastDState))
			{
				// A single direction is pressed and it is the same as before...
				m_fQSTimer += FLoop_fPreviousLoopSecs;
				if(m_fQSTimer >= 0.125f)
				{
					m_nDrawDir = m_nLastDir;
					m_eQSState = QSSTATE_CLEANSTARTED;
				}
			}
			else
			{
/*				if(nCurDir == -1)
					DEVPRINTF("Multiple directions down.\n");
				if((uCurDState != m_uLastDState) && (m_uLastDState != 0))
					DEVPRINTF("Directions have changed.\n");*/
				m_fQSTimer = 0.0f;
			}

			m_uLastDState = uCurDState;
			m_nLastDir = nCurDir;

			break;
		}
		case QSSTATE_CLEANSTARTED:
		{
			FASSERT(m_nLastDir != -1);
			if(uCurDState == 0)
			{
				// The direction has been released.
//				DEVPRINTF("LoadB [%d].\n", m_nLastDir);
				LoadProfile(m_nLastDir, pInventory);
				m_fQSTimer += FLoop_fPreviousLoopSecs;
				m_eQSState = QSSTATE_FINISHINGARROWDISPLAY;
			}
			else if((uCurDState & (1 << m_nLastDir)) == 0)
			{
				// A direction is still pressed, but it's not the right one.
//				DEVPRINTF("Direction lost.\n");

				// We could possibly do something else here.
				m_nLastDir = -1;
				m_nDrawDir = -1;
				m_uLastDState = 0;
				m_fQSTimer = 0.0f;
				m_eQSState = QSSTATE_WAITINGFORCLEANSTART;
			}
			else
			{
				m_fQSTimer += FLoop_fPreviousLoopSecs;
				if(m_fQSTimer >= 1.0f)
				{
//					DEVPRINTF("Save [%d].\n", m_nLastDir);
					SaveProfile(m_nLastDir, pInventory);

					m_fQSTimer = 0.0f;
					m_eQSState = QSSTATE_ARROWFLASHING;
				}
			}

			break;
		}
		case QSSTATE_FINISHINGARROWDISPLAY:
		{
			m_fQSTimer += FLoop_fPreviousLoopSecs;
			if(nCurDir != -1)
			{
				m_nLastDir = nCurDir;
				m_nDrawDir = nCurDir;
				m_uLastDState = uCurDState;
				m_fQSTimer = 0.0f;
				m_eQSState = QSSTATE_WAITINGFORCLEANSTART;
			}
			else if(m_fQSTimer >= 0.5f)
			{
				m_nDrawDir = -1;
				m_nLastDir = -1;
				m_uLastDState = 0;
				m_fQSTimer = 0.0f;
				m_eQSState = QSSTATE_WAITINGFORCLEANSTART;
			}

			break;
		}
		case QSSTATE_ARROWFLASHING:
		{
			m_fQSTimer += FLoop_fPreviousLoopSecs;
			if(m_fQSTimer >= fQSFlashDuration)
			{
				m_eQSState = QSSTATE_WAITINGFORRELEASE;
			}
			else
			{
				m_uWhichArrow = fmath_FloatToU32(m_fQSTimer * fQSArrowFlashFreq) & 1;
/*				if(m_fQSTimer < 0.5f)
					m_uWhichArrow = 0;
				else if(m_fQSTimer < 1.0f)
					m_uWhichArrow = 1;
				else if(m_fQSTimer < 1.5f)
					m_uWhichArrow = 0;
				else
					m_uWhichArrow = 1;*/
			}

			break;
		}
		case QSSTATE_WAITINGFORRELEASE:
		{
			if(uCurDState == 0)
			{
				m_nDrawDir = -1;
				m_nLastDir = -1;
				m_uLastDState = 0;
				m_fQSTimer = 0.0f;
				m_eQSState = QSSTATE_WAITINGFORCLEANSTART;
			}
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================
// QuickCycle replaces QuickSelect for multiplayer. A quick button press
// will just cycle to the next/previous weapon instantly.
void CHud2::QuickCycleWork(CInventory *pInventory)
{
	if ( !m_bWSEnabled )
		return;

	s32 nController = Player_aPlayer[m_nPlayerIdx].m_nControllerIndex;

	if (Gamepad_aapSample[nController][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT)
		pInventory->CycleToNextWeapon(0, FALSE);
	if (Gamepad_aapSample[nController][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT)
		pInventory->CycleToNextWeapon(1, FALSE);

	// In case we want to resurrect the D pad, the code appears below in the comment...

	//// Get up/down value
	//if (Gamepad_aapSample[nController][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT) {
	//	if (Gamepad_aapSample[nController][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState > 0.0f)
	//		pInventory->CycleToNextWeapon(0, FALSE);
	//	else
	//		pInventory->CycleToPrevWeapon(0, FALSE);
	//}
	//
	//// Left/Right
	//if (Gamepad_aapSample[nController][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT) {
	//	if (Gamepad_aapSample[nController][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState > 0.0f)
	//		pInventory->CycleToNextWeapon(1, FALSE);
	//	else
	//		pInventory->CycleToPrevWeapon(1, FALSE);
	//}
}

// =============================================================================================================

void CHud2::QSDraw2()
{
	switch(m_eQSState)
	{
		case QSSTATE_WAITINGFORCLEANSTART:
		case QSSTATE_WAITINGFORRELEASE:
		{
			break;
		}
		case QSSTATE_CLEANSTARTED:
		case QSSTATE_FINISHINGARROWDISPLAY:
		{
			u32 uCurIdx;
			FASSERT(m_nDrawDir != -1);
			for(uCurIdx = 0; uCurIdx < 4; ++uCurIdx)
			{
				m_aavtxArrow[m_nDrawDir][uCurIdx].ColorRGBA.Set( 0.25f, 0.25f, 1.0f, 1.0f );
			}
			fdraw_SetTexture(&_texHud);
			fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, m_aavtxArrow[m_nDrawDir], 4);
			break;
		}
		case QSSTATE_ARROWFLASHING:
		{
			u32 uCurIdx;
			FASSERT(m_nDrawDir != -1);
			for(uCurIdx = 0; uCurIdx < 4; ++uCurIdx)
			{
				m_aavtxArrow[m_nDrawDir][uCurIdx].ColorRGBA = argbaArrowFlash[m_uWhichArrow];
			}
			fdraw_SetTexture(&_texHud);
			fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, m_aavtxArrow[m_nDrawDir], 4);
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================

void CHud2::WSBoxSetDest(f32 fDest)
{
	m_fWSBoxDestBPUnitPos = fDest;

	if(m_fWSBoxBPUnitPos == fDest)
	{
		// We're already there, let's just make sure that we don't go anywhere.
		m_fWSBoxDir = 0.0f;
		return;
	}

	if(fDest > m_fWSBoxBPUnitPos)
	{
		// We need to move forward.
		m_fWSBoxDir = 1.0f;
		return;
	}

	if(fDest < m_fWSBoxBPUnitPos)
	{
		// We need to move backward.
		m_fWSBoxDir = -1.0f;
		return;
	}

	FASSERT_NOW;
}

// =============================================================================================================

// Returns NULL if ammo level is ok.
// Otherwise, returns a pointer to "Low Ammo" or "No Ammo".
//
// Fills *pnDeciRed, *pnDeciGreen, and *pnDeciBlue with values from 0 to 99, representative
// of the color to draw the ammo alert text (all three are 99 if ammo is ok).
cwchar *CHud2::ComputeAmmoAlertTextColor( CWeapon *pWeapon, u32 *pnDeciRed, u32 *pnDeciGreen, u32 *pnDeciBlue )
{
	if( pWeapon ) {
		CWeapon::AmmoAlert_e nAmmoAlert = pWeapon->ComputeAmmoAlert();

		if( nAmmoAlert != CWeapon::AMMO_ALERT_OK ) {
			cwchar *pwszText;

			u32 nDeciIntensity = (u32)( 99.0f * (GetAmmoAlertUnitCountdownTimer() * 0.75f + 0.245f) );

			if( nAmmoAlert == CWeapon::AMMO_ALERT_LOW_AMMO ) {
				pwszText = Game_apwszPhrases[ GAMEPHRASE_RETICLE_LOW_AMMO ];

				*pnDeciRed = (nDeciIntensity >> 1) + 50;
				*pnDeciGreen = (nDeciIntensity >> 1) + 50;
				*pnDeciBlue = nDeciIntensity;
			} else {
				pwszText = Game_apwszPhrases[ GAMEPHRASE_RETICLE_NO_AMMO ];

				*pnDeciRed = 99;
				*pnDeciGreen = nDeciIntensity;
				*pnDeciBlue = nDeciIntensity;
			}

			return pwszText;
		}
	}

	*pnDeciRed = 99;
	*pnDeciGreen = 99;
	*pnDeciBlue = 99;

	return NULL;
}


void CHud2::WSBoxDrawFDraw(CHudWSItem *pItem, CHudWSItem *pOppositeItem)
{
	if(m_fWSBoxBPUnitPos == 0.0f)
	{
		return;
	}

	FDrawVtx_t avtxBox[4];

	f32 fOffsetFactor = 1.0f - 2.0f * (f32)(m_nWhichWeaponSelectIsActive);

	f32 fX1 = fWSInfoBoxUL.x;// * fOffsetFactor;
	f32 fY1 = fWSInfoBoxUL.y;
	f32 fX2 = fWSInfoBoxLR.x;// * fOffsetFactor;
	f32 fY2 = fWSInfoBoxLR.y;

	fdraw_SetTexture(&_texInfoBox);

	f32 fS1 = CHud2_vecInfoBoxULST.x;
	f32 fT1 = CHud2_vecInfoBoxULST.y;
	f32 fS2 = CHud2_vecInfoBoxLRST.x;
	f32 fT2 = CHud2_vecInfoBoxLRST.y;

	const f32 fZ = 0.0f;

	avtxBox[0].ColorRGBA.OpaqueWhite();
	avtxBox[0].Pos_MS.Set(fX1, fY1, fZ);
	avtxBox[0].ST.Set(fS1, fT1);

	avtxBox[1].ColorRGBA.OpaqueWhite();
	avtxBox[1].Pos_MS.Set(fX2, fY1, fZ);
	avtxBox[1].ST.Set(fS2, fT1);

	avtxBox[2].ColorRGBA.OpaqueWhite();
	avtxBox[2].Pos_MS.Set(fX2, fY2, fZ);
	avtxBox[2].ST.Set(fS2, fT2);	

	avtxBox[3].ColorRGBA.OpaqueWhite();
	avtxBox[3].Pos_MS.Set(fX1, fY2, fZ);
	avtxBox[3].ST.Set(fS1, fT2);	

	CFXfm xfmRotate;
	xfmRotate.BuildRotationY(m_fWSBoxCurThetaY, 0.0f, 0.0f, 1.0f);

	xfmRotate.PushModel();

	fdraw_PrimList(FDRAW_PRIMTYPE_QUADLIST, avtxBox, 4);

	CFXfm::PopModel();

	// If we aren't paused on a weapon, don't write any text.
	if((m_eWeaponSelectState != WEAPONSELECTSTATE_PAUSED) && (m_eWeaponSelectState != WEAPONSELECTSTATE_SCROLLINGTO))
	{
		return;
	}

	if(FMATH_FABS(m_fWSBoxBPUnitPos) != 1.0f)
	{
		return;
	}

	FASSERT(m_nWhichWeaponSelectIsActive != -1);
	u32 uCurItem = m_auWSCurSelected[m_nWhichWeaponSelectIsActive];

	CItemInst *pItemInst = m_aWSItem[m_nWhichWeaponSelectIsActive][uCurItem].m_pItemInst;

	CItem *pTempItem = pItemInst->m_pItemData;
	CWeapon *pWeapon = pItemInst->m_pWeapon;

	{
		Hud2_XFormPrintf( m_ahWSName[m_nWhichWeaponSelectIsActive], L"~C80809999%ls", pTempItem->m_pwszDisplayName );

		Hud2_XFormPrintf( m_ahWSDesc[m_nWhichWeaponSelectIsActive], L"~C60609999~o1%ls", pTempItem->m_pwszLongDesc);

		if( pTempItem->m_uCurLevel )
		{
			Hud2_XFormPrintf(m_ahWSEUK[m_nWhichWeaponSelectIsActive], L"~C40994099~o1%ls %d", Game_apwszPhrases[ GAMEPHRASE_EUK_LEVEL ], pTempItem->m_uCurLevel);
		}

		u32 nDeciRed, nDeciGreen, nDeciBlue;
		cwchar *pwszText = ComputeAmmoAlertTextColor( pWeapon, &nDeciRed, &nDeciGreen, &nDeciBlue );
		if( pwszText ) {
			// Low or no ammo...
			Hud2_XFormPrintf( m_ahWSAmmoAlert[m_nWhichWeaponSelectIsActive], L"~C%02u%02u%02u99%ls", nDeciRed, nDeciGreen, nDeciBlue, pwszText );
		} else {
			if( (GetAmmoAlertUnitCountdownTimer() < 0.75f) && CHudWSItem::ShouldWeDisplayScopeUnusableInfo( pItem, pOppositeItem ) ) {
				Hud2_XFormPrintf( m_hScopeUnusableTextArea, L"~C99990099~o1%ls", Game_apwszPhrases[ GAMEPHRASE_SCOPE_INCOMPATIBLE ] );
			}
		}

		// Draw upgradeable text...
		if( pItemInst->m_pItemData->m_uMaxLevel ) {
			if( pItemInst->m_pItemData->m_uCurLevel < pItemInst->m_pItemData->m_uMaxLevel ) {
				f32 fTextScale = fmath_Sin( CHud2::GetAmmoAlertUnitCountdownTimer() * FMATH_2PI ) * 0.05f + 1.0f;
				Hud2_XFormPrintf( m_ahWSUpgradeable[m_nWhichWeaponSelectIsActive], L"~C40994099~S%1.2f%ls", fTextScale, Game_apwszPhrases[ GAMEPHRASE_UPGRADABLE ] );
			}
		}

		switch(pTempItem->m_uFlags)
		{
			case ITEMFLAG_PRIMARY:
			{
				Hud2_XFormPrintf(m_ahWSWeapType[m_nWhichWeaponSelectIsActive], L"%ls\n  %ls", 
					Game_apwszPhrases[ GAMEPHRASE_PRIMARY ], Game_apwszPhrases[ GAMEPHRASE_WEAPON ] );
				break;
			}
			case ITEMFLAG_SECONDARY:
			{
				Hud2_XFormPrintf(m_ahWSWeapType[m_nWhichWeaponSelectIsActive], L"%ls\n  %ls",
					Game_apwszPhrases[ GAMEPHRASE_SECONDARY ], Game_apwszPhrases[ GAMEPHRASE_WEAPON ] );
				break;
			}
			case ITEMFLAG_DUAL:
			{
				Hud2_XFormPrintf(m_ahWSWeapType[m_nWhichWeaponSelectIsActive], L"%ls\n  %ls",
					Game_apwszPhrases[ GAMEPHRASE_DUAL ], Game_apwszPhrases[ GAMEPHRASE_WEAPON ] );
				break;
			}
			default:
			{
				FASSERT_NOW;
			}
		}
	}
}

// =============================================================================================================

void CHud2::WSBoxDrawMesh(CHudWSItem *pItem)
{
	if(FMATH_FABS(m_fWSBoxBPUnitPos) != 1.0f)
	{
		return;
	}

	FASSERT(m_nWhichWeaponSelectIsActive != -1);

	CItemInst *pII = pItem->m_pItemInst;
	FASSERT(pII != NULL);
	CItem *pI = (CItem *)(pII->m_pItemData);
	FASSERT(pI != NULL);

	pI->DrawMesh(m_fWSItemTheta);
}

// =============================================================================================================

void CHud2::WSBoxWork()
{
	if(m_fWSBoxDir == 0.0f)
	{
		return;
	}

	m_fWSBoxBPUnitPos += CHud2_fOOWSBoxTime * m_fWSBoxDir * FLoop_fRealPreviousLoopSecs;

	if(m_fWSBoxDir == -1.0f)
	{
		// Check to see if we've overshot.
		if(m_fWSBoxBPUnitPos < m_fWSBoxDestBPUnitPos)
		{
			m_fWSBoxBPUnitPos = m_fWSBoxDestBPUnitPos;
			m_fWSBoxDir = 0.0f;
		}
	}
	else if(m_fWSBoxDir == 1.0f)
	{
		// Check to see if we've overshot.
		if(m_fWSBoxBPUnitPos > m_fWSBoxDestBPUnitPos)
		{
			m_fWSBoxBPUnitPos = m_fWSBoxDestBPUnitPos;
			m_fWSBoxDir = 0.0f;
		}
	}

	m_fWSBoxCurThetaY = m_fWSBoxBPUnitPos * FMATH_HALF_PI - FMATH_HALF_PI;
}

// So on a checkpoint restore we get rid of the spinning 3D item above our head
void CHud2::CheckpointRestore( void ) {
	for( u32 i = 0; i < 3; ++i ) {
		if( m_aoHudItemInst[i].m_eItemState != ITEMSTATE_EMPTY ) {
			m_aoHudItemInst[i].Empty();
		}
	}

	TransmissionMsg_Stop( FALSE );
	_TextDisplayRelease();

	ResetRadar();
}

//
//
//
BOOL CHud2::TextDisplay_Start( f32 fDisplayTime, CFColorRGBA *pColor, BOOL bBlink, BOOL bPulse, cwchar *pszWFormat, ... ) {
	if( fDisplayTime <= 0.0f ) {
		DEVPRINTF( "CHud2::TextDisplay_Start(): Invalid display time '%f'.\n", fDisplayTime );
		return FALSE;
	}

	if( m_eTextDisplayState != TEXTDISPLAY_STATE_IDLE ) {
		_TextDisplayRelease();
	}

	FDrawVtx_t *pVerts = NULL;
	u32 uNumVertsNeeded;
	va_list	oVAList;

	uNumVertsNeeded = ( ( sizeof( wchar ) * Fang_ConfigDefs.nText_MaxCharsPerPrintf ) / sizeof( FDrawVtx_t ) );

	if( ( ( sizeof( wchar ) * Fang_ConfigDefs.nText_MaxCharsPerPrintf ) % sizeof( FDrawVtx_t ) ) ) {
		++uNumVertsNeeded;
	}

	if( !uNumVertsNeeded ) {
		DEVPRINTF( "CHud2::TextDisplay_Start(): Fang_ConfigDefs.nText_MaxCharsPerPrintf is 0.\n" );
		return FALSE;
	}

	pVerts = fvtxpool_GetArray( uNumVertsNeeded );

	if( !pVerts ) {

		return FALSE;
	}

	// Output the string
	va_start( oVAList, pszWFormat );
	if( _vsnwprintf( (wchar* ) pVerts, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszWFormat, oVAList ) == -1 ) {
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	m_pTextBuffer = (wchar *) pVerts;
	m_eTextDisplayState = TEXTDISPLAY_STATE_ON;
	m_fTextDisplayTime = fDisplayTime;
	m_fTextFlashTime = bBlink ? _TEXTDISPLAY_FLASH_SECS : 0.0f;
	
	if( bPulse ) {
		m_fTextPulseAngle = 0.0001f; // Make it non-zero
	} else {
		m_fTextPulseAngle = 0.0f;
	}

	if( !pColor ) {
		m_TextColor.OpaqueWhite();
	} else {
		m_TextColor = *pColor;
	}

	return TRUE;
}

void CHud2::_TextDisplayRelease( void ) {
	if( !m_pTextBuffer ) {
		return;
	}

	fvtxpool_ReturnArray( (FDrawVtx_t *) m_pTextBuffer );
	m_pTextBuffer = NULL;
	m_eTextDisplayState = TEXTDISPLAY_STATE_IDLE;
}

void CHud2::_TextDisplayWork( void ) {
	switch( m_eTextDisplayState ) {
		case TEXTDISPLAY_STATE_IDLE:
			return;
		break;
		case TEXTDISPLAY_STATE_ON:
			// If m_fTextFlashTime == 0.0f, no flashing
			if( m_fTextFlashTime > 0.0f ) {
				m_fTextFlashTime -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fTextFlashTime, 0.0f );

				if( m_fTextFlashTime == 0.0f ) {
					m_fTextFlashTime = -_TEXTDISPLAY_FLASH_SECS;
				}
			} else if( m_fTextFlashTime < 0.0f ) {
				m_fTextFlashTime += FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fTextFlashTime, 0.0f );

				if( m_fTextFlashTime == 0.0f ) {
					m_fTextFlashTime = _TEXTDISPLAY_FLASH_SECS;
				}
			}

			m_fTextDisplayTime -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fTextDisplayTime, 0.0f );

			if( m_fTextDisplayTime == 0.0f ) {
				m_fTextDisplayTime = _TEXTDISPLAY_FADEOUT_TIME;
				m_eTextDisplayState = TEXTDISPLAY_STATE_FADING_OUT;
			}
		break;
		case TEXTDISPLAY_STATE_FADING_OUT:
			m_fTextDisplayTime -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fTextDisplayTime, 0.0f );

			if( m_fTextDisplayTime == 0.0f ) {
				_TextDisplayRelease();
				m_eTextDisplayState = TEXTDISPLAY_STATE_IDLE;
			}			
		break;
	}

	if( m_fTextPulseAngle != 0.0f ) {
		m_fTextPulseAngle += ( 2.0f * FMATH_2PI * FLoop_fPreviousLoopSecs );
	}
}
//
//
//

// =============================================================================================================

f32 JCXToFTextX(f32 fX)
{
	return((fX + 1.0f) * 0.5f);
}

// =============================================================================================================

f32 JCYToFTextY(f32 fY)
{
	return((-fY + 0.75f) * 0.5f);
}

// =============================================================================================================

f32 FTextXToJCX(f32 fX)
{
	return(2.0f * fX - 1.0f);
}

f32 FTextYToJCX(f32 fY)
{
	return(-2.0f * fY + .75f);
}

void CHud2::_PrepareAmmoDisplay( CInventory *pInventory, u32 uSide, FDrawVtx_t *pvtxArray, u32 *puStartVtx ) {
	FASSERT( uSide < 2 );

	switch( m_auAmmoOverrideMode[uSide] ) {
		case OVERRIDE_NONE:
			if( !pInventory ) {
				break;
			}

			if(m_eCurHudMode == HUDMODE_MIL)
			{
				pInventory->m_aoWeapons[uSide][m_auWSCurSelected[uSide]].PrintItemText(m_ahCurAmmo[uSide], 0);
			}
			else
			{
				pInventory->m_aoWeapons[uSide][m_auWSCurSelected[uSide]].PrintItemText(m_ahCurAmmo[uSide], m_ahMaxAmmo[uSide], pvtxArray, puStartVtx, TRUE, FALSE );
			}
			break;

		case OVERRIDE_COUNTER:
			FASSERT( DrawFlagsEnabled(DRAW_ENABLEOVERRIDE) );
			FASSERT( m_apfAmmoOverrideData[uSide] != NULL );
			
			if( m_ahCurAmmo[uSide] != 0 ) {
				u32 uValue = (u32)*m_apfAmmoOverrideData[uSide];
				if( uValue > 999 ) {
					uValue = 0;
				}
				Hud2_XFormPrintf(m_ahCurAmmo[uSide], "%d", uValue);
			}
			break;

		case OVERRIDE_METER:
			FASSERT( DrawFlagsEnabled(DRAW_ENABLEOVERRIDE) );
            if((pvtxArray != NULL) && (puStartVtx != NULL) ) {
				FASSERT( m_apfAmmoOverrideData[uSide] != NULL );
				FASSERT_UNIT_FLOAT( *m_apfAmmoOverrideData[uSide] );

				static const f32 _fRS1 = 0.0078125f;
				static const f32 _fT1 = 0.921875f;
				static const f32 _fRS2 = 0.23632812f;
				static const f32 _fT2 = 0.9921875f;

				static const f32 _fRX1 = 0.318f;
				static const f32 _fRY1 = -0.50f;
				static const f32 _fRY2 = -0.605f;
				static const f32 _fRWidth = 0.360f;

				static const f32 _fLX1 = -0.405f;  //.51
				static const f32 _fLY1 = -0.51f;
				static const f32 _fLY2 = -0.595f;
				static const f32 _fLWidth = 0.280f;

				f32 _fX1, _fY1, fX2, _fY2, _fS1, fS2;

				f32 fUnitAmmo = *m_apfAmmoOverrideData[uSide];

				if( uSide == 0 ) {
					_fX1 = _fRX1;
					_fY1 = _fRY1;
					fX2 = _fRX1 + (_fRWidth * fUnitAmmo);
					_fY2 = _fRY2;
					_fS1 = _fRS1;
					fS2 = _fRS1 + (fUnitAmmo) * (_fRS2 - _fRS1);
				} else {
					_fX1 = _fLX1;
					_fY1 = _fLY1;
					fX2 = _fLX1 - (_fLWidth * fUnitAmmo);
					_fY2 = _fLY2;
					_fS1 = _fRS1;
					fS2 = _fRS1 + (fUnitAmmo) * (_fRS2 - _fRS1);
				}

				CFColorRGBA rgbaColor; 

				if( !m_abAmmoOverrideColor[uSide] ) {
					rgbaColor.fRed		= fUnitAmmo > 0.5f ? (2.0f - 2.0f * fUnitAmmo) : 1.0f;
					rgbaColor.fGreen	= fUnitAmmo < 0.5f ? (2.0f * fUnitAmmo) : 1.0f;
					rgbaColor.fBlue		= 0.0f;
					rgbaColor.fAlpha	= 1.0f;
				} else {
					f32 fUnitColor = fmath_UnitLinearToSCurve( fUnitAmmo );

					rgbaColor.fRed		= m_aAmmoOverrideStartColorRGB[uSide].fRed		+ (m_aAmmoOverrideEndColorRGB[uSide].fRed		- m_aAmmoOverrideStartColorRGB[uSide].fRed)		* fUnitColor;
					rgbaColor.fGreen	= m_aAmmoOverrideStartColorRGB[uSide].fGreen	+ (m_aAmmoOverrideEndColorRGB[uSide].fGreen	- m_aAmmoOverrideStartColorRGB[uSide].fGreen)	* fUnitColor;
					rgbaColor.fBlue		= m_aAmmoOverrideStartColorRGB[uSide].fBlue		+ (m_aAmmoOverrideEndColorRGB[uSide].fBlue		- m_aAmmoOverrideStartColorRGB[uSide].fBlue)	* fUnitColor;
					rgbaColor.fAlpha	= m_aAmmoOverrideStartColorRGB[uSide].fAlpha	+ (m_aAmmoOverrideEndColorRGB[uSide].fAlpha		- m_aAmmoOverrideStartColorRGB[uSide].fAlpha)	* fUnitColor;
				}

				// Upper left.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(_fX1, _fY1, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(_fS1, _fT1);

				// Upper right.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(fX2, _fY1, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(fS2, _fT1);

				// Lower right.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(fX2, _fY2, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(fS2, _fT2);

				// Lower left.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(_fX1, _fY2, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(_fS1, _fT2);
			}
			break;

		case OVERRIDE_METER_MIL:
			FASSERT( DrawFlagsEnabled(DRAW_ENABLEOVERRIDE) );
            if((pvtxArray != NULL) && (puStartVtx != NULL) ) {
				
				FASSERT( m_apfAmmoOverrideData[uSide] != NULL );
				FASSERT_UNIT_FLOAT( *m_apfAmmoOverrideData[uSide] );

				static const f32 _fRS1 = 0.482421875;
				static const f32 _fT1 = 0.83984375;
				static const f32 _fRS2 = 0.630859375;
				static const f32 _fT2 = 0.900390625;

				static const f32 _fRX1 = CHud2_avecAmmoBoxUL[0].x + 0.106875f; // 0.41f;
				static const f32 _fRY1 = CHud2_avecAmmoBoxUL[0].y - 0.0195f; // -0.507f;
				static const f32 _fRY2 = _fRY1 - 0.091f; //-0.598f;
				static const f32 _fRWidth = 0.2375f;

//				static const f32 _fLX1 = -0.505f;  //.51
//				static const f32 _fLWidth = 0.1425f;

				f32 _fLX1 = CHud2_avecAmmoBoxUL[1].x + 0.211875f; //-0.46f; 
				f32 _fLWidth = 0.1938f;

				//RAF OLD!
//				static const f32 _fLY1 = -0.507f;
//				static const f32 _fLY2 = -0.598f;
				//RAF NEW
				static const f32 _fLY1 = CHud2_avecAmmoBoxUL[1].y - 0.0195f; // -0.507f;
				static const f32 _fLY2 = _fLY1 - 0.091f; //-0.598f;


				f32 _fX1, _fY1, fX2, _fY2, _fS1, fS2;

				f32 fUnitAmmo = *m_apfAmmoOverrideData[uSide];

				if( uSide == 0 ) {
					_fX1 = _fRX1;
					_fY1 = _fRY1;
					fX2 = _fRX1 + (_fRWidth * fUnitAmmo);
					_fY2 = _fRY2;
					_fS1 = _fRS1;
					fS2 = _fRS1 + (fUnitAmmo) * (_fRS2 - _fRS1);
				} else {
					_fX1 = _fLX1;
					_fY1 = _fLY1;
					fX2 = _fLX1 - (_fLWidth * fUnitAmmo);
					_fY2 = _fLY2;
					_fS1 = _fRS1;
					fS2 = _fRS1 + (fUnitAmmo) * (_fRS2 - _fRS1);
				}

				CFColorRGBA rgbaColor; 

				if( !m_abAmmoOverrideColor[uSide] ) {
					rgbaColor.fRed		= fUnitAmmo > 0.5f ? (2.0f - 2.0f * fUnitAmmo) : 1.0f;
					rgbaColor.fGreen	= fUnitAmmo < 0.5f ? (2.0f * fUnitAmmo) : 1.0f;
					rgbaColor.fBlue		= 0.0f;
					rgbaColor.fAlpha	= 1.0f;
				} else {
					f32 fUnitColor = fmath_UnitLinearToSCurve( fUnitAmmo );

					rgbaColor.fRed		= m_aAmmoOverrideStartColorRGB[uSide].fRed		+ (m_aAmmoOverrideEndColorRGB[uSide].fRed		- m_aAmmoOverrideStartColorRGB[uSide].fRed)		* fUnitColor;
					rgbaColor.fGreen	= m_aAmmoOverrideStartColorRGB[uSide].fGreen	+ (m_aAmmoOverrideEndColorRGB[uSide].fGreen	- m_aAmmoOverrideStartColorRGB[uSide].fGreen)	* fUnitColor;
					rgbaColor.fBlue		= m_aAmmoOverrideStartColorRGB[uSide].fBlue		+ (m_aAmmoOverrideEndColorRGB[uSide].fBlue		- m_aAmmoOverrideStartColorRGB[uSide].fBlue)	* fUnitColor;
					rgbaColor.fAlpha	= m_aAmmoOverrideStartColorRGB[uSide].fAlpha	+ (m_aAmmoOverrideEndColorRGB[uSide].fAlpha		- m_aAmmoOverrideStartColorRGB[uSide].fAlpha)	* fUnitColor;

				}

				// Upper left.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(_fX1, _fY1, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(_fS1, _fT1);

				// Upper right.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(fX2, _fY1, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(fS2, _fT1);

				// Lower right.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(fX2, _fY2, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(fS2, _fT2);

				// Lower left.
				pvtxArray[(*puStartVtx)].Pos_MS.Set(_fX1, _fY2, 0.0f);
				pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
				pvtxArray[(*puStartVtx)++].ST.Set(_fS1, _fT2);
			}
			break;

		case OVERRIDE_COUNTER_MIL:
			FASSERT( DrawFlagsEnabled(DRAW_ENABLEOVERRIDE) );
			FASSERT( m_apfAmmoOverrideData[uSide] != NULL );
			
			if( m_ahCurAmmo[uSide] != 0 ) {
				u32 uValue = (u32)*m_apfAmmoOverrideData[uSide];
				if( uValue > 999 ) {
					uValue = 0;
				}
				Hud2_XFormPrintf(m_ahCurAmmo[uSide], "%d", uValue);
			}
			break;

		default:
			FASSERT_NOW;
	}
}

static void _XFormTextArea(const FTextAreaHandle_t ohArea, f32* afSavedCoords)
{
	FTextArea_t* pT = ftext_GetAttributes(ohArea);
	FViewport_t* pVP = fviewport_GetActive();

	// Save the old coordinates so we don't keep changing ones that aren't updated
	afSavedCoords[0] = pT->fUpperLeftX;
	afSavedCoords[1] = pT->fUpperLeftY;
	afSavedCoords[2] = pT->fLowerRightX;
	afSavedCoords[3] = pT->fLowerRightY;

	// Get the corners of the text area in our screen coordinates
	CFVec3 vUL(FTextXToJCX(pT->fUpperLeftX), FTextYToJCX(pT->fUpperLeftY), 0.0f);
	CFVec3 vLR(FTextXToJCX(pT->fLowerRightX), FTextYToJCX(pT->fLowerRightY), 0.0f);

	// Transform them by the current model matrix
	FXfm_pModel->TransformPointF(vUL, vUL);
	FXfm_pModel->TransformPointF(vLR, vLR);

	// Tranform that back into text space, X on [0,1], Y on [0,0.75]
	pT->fUpperLeftX  = (vUL.x + pVP->HalfRes.x) * pVP->OORes.x;
	pT->fLowerRightX = (vLR.x + pVP->HalfRes.x) * pVP->OORes.x;
	pT->fUpperLeftY  = (pVP->HalfRes.y - vUL.y) * pVP->OORes.y * 0.75f;
	pT->fLowerRightY = (pVP->HalfRes.y - vLR.y) * pVP->OORes.y * 0.75f;
}

static void _RestoreTextArea(const FTextAreaHandle_t ohArea, f32* afSavedCoords)
{
	FTextArea_t* pT  = ftext_GetAttributes(ohArea);
	pT->fUpperLeftX  = afSavedCoords[0];
	pT->fUpperLeftY  = afSavedCoords[1];
	pT->fLowerRightX = afSavedCoords[2];
	pT->fLowerRightY = afSavedCoords[3];
}

// Local version of ftext_PrintString that transforms everything correctly first
void Hud2_PrintString( const FTextAreaHandle_t ohArea, cwchar *pszWString )
{
	f32 afSavedCoords[4];

	_XFormTextArea( ohArea, afSavedCoords );
	ftext_PrintString( ohArea, pszWString );
	_RestoreTextArea( ohArea, afSavedCoords );
}

void Hud2_XFormPrintf( const FTextAreaHandle_t ohArea, cwchar *pszWFormat, ... )
{
	// transform the text area
	f32 afSavedCoords[4];
	_XFormTextArea( ohArea, afSavedCoords );

	va_list	pArgs;

	va_start( pArgs, pszWFormat );
	wchar tmp[160];
	_vsnwprintf( tmp, 160, pszWFormat, pArgs );
	ftext_Printf( ohArea, tmp );
	va_end( pArgs );

	// Restore the text area
	_RestoreTextArea( ohArea, afSavedCoords );
}

void Hud2_XFormPrintf( const FTextAreaHandle_t ohArea, cchar *pszFormat, ... )
{
	// transform the text area
	f32 afSavedCoords[4];
	_XFormTextArea( ohArea, afSavedCoords );

	va_list	pArgs;

	va_start( pArgs, pszFormat );
	char tmp[40];
	_vsnprintf( tmp, 40, pszFormat, pArgs );
	ftext_Printf( ohArea, tmp );
	va_end( pArgs );

	// Restore the text area
	_RestoreTextArea( ohArea, afSavedCoords );
}

// This one takes HUD coordinates instead of text coordinates, transforms
// them by the current model matrix, converts to text coordinates, and passes
// it all along to ftext.
static void _XFormPrintfLocal( f32 fX, f32 fY, cchar *pszFormat, ... )
{
	CFVec3 v(fX, fY, 0.0f);
	FXfm_pModel->TransformPointF(v, v);

	FViewport_t* pVP = fviewport_GetActive();
	fX = (v.x + pVP->HalfRes.x) * pVP->OORes.x;
	fY = (pVP->HalfRes.y - v.y) * pVP->OORes.y * 0.75f;
	
	va_list pArgs;
	va_start( pArgs, pszFormat );
	char tmp[40];
	_vsnprintf( tmp, 40, pszFormat, pArgs );
	ftext_Printf( fX, fY, tmp );
	va_end( pArgs	);
}

