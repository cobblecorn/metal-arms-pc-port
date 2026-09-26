//////////////////////////////////////////////////////////////////////////////////////
// PauseScreen.h - Pause Screen system for Mettle Arms.
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
// 02/21/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _PAUSESCREEN_H_
#define _PAUSESCREEN_H_ 1

#include "fang.h"
#include "MenuTypes.h"
#include "ItemInst.h"

// these define are used in the hud2 module as well, why duplicate them?
#if FANG_PLATFORM_DX
	const static char *CPauseScreen_pszControlsTex = "tfh2bttnsxb";
#elif FANG_PLATFORM_GC
	const static char *CPauseScreen_pszControlsTex = "tfh2bttnsgc";
#elif FANG_PLATFORM_PS2
	//ARG - we use XBox tex name and replace in pasm - may change to use this feature
	const static char *CPauseScreen_pszControlsTex = "tfh2bttnsxb";
#endif

typedef enum {
	PAUSESCREEN_BUTTON_ST_A = 0,
	PAUSESCREEN_BUTTON_ST_B,
	PAUSESCREEN_BUTTON_ST_L,
	PAUSESCREEN_BUTTON_ST_R,
	PAUSESCREEN_BUTTON_ST_Y,	
	PAUSESCREEN_BUTTON_ST_X,
	PAUSESCREEN_BUTTON_ST_START,

	PAUSESCREEN_BUTTON_ST_COUNT
} PauseScreen_ButtonsST_e;

extern const CFVec2 CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_COUNT];
extern const CFVec2 CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_COUNT];


class CInventory;
struct FViewport_t;

class CMenuHighlight
{
public:
	CMenuHighlight();
	~CMenuHighlight();

	void Draw() const;

	CFVec3 m_vecUL, m_vecRect;
	BOOL m_bIsActive;
	f32 m_fAlpha;
	static CFTexInst m_tex;

	FCLASS_STACKMEM_NOALIGN(CMenuHighlight);
};

enum PauseScreenState_e
{
	PSSTATE_INACTIVE,
	PSSTATE_NORMAL,
};

class CPauseScreen  
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL LevelInit();
	static void LevelUninit();

	static BOOL IsActive();
	static void SetEnabled(BOOL bEnabled);
	static BOOL Start(CInventory *pInventory );
	static BOOL ExitPause( void );
	static void SetWSEnable(BOOL bWSEnabled);

	static void Work(CInventory *pInventory);
	static void Draw(CInventory *pInventory);

	static void GetControls();
	FINLINE static void EnableAudioPause(BOOL bEnable) { m_bPauseAudio = bEnable; }

private:
	static void SetupFrame();
	static void DrawFrame();
	static void WeaponCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData);
//	static void YesNoButtonCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData);
	static void CancelOnlyCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData);
	static void OptionScreenCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData);
	static void DrawInfoBoxMesh(CMenuItem *pMI);
	static void DrawInfoBoxText(CMenuItem *pMI, CInventory *pInventory);
	static void DrawMissionText( void );
	static BOOL IsUpgradeableNow(CMenuItem *pMI, CInventory *pInventory);
	static BOOL IsANullItem(CItemInst *pII);
	static void RefreshSelected(CInventory *pInventory);
	static void _SetTextItemDraw(u32 uWhichWeapon, BOOL bDrawText);

	static void DrawScreenTitle();

	static u32 m_uButtons, m_uButtonsLatched;

	static FViewport_t *m_pviewOrtho3d;

	static CItemInst m_aoNullSlot[3];

	static CMenuItem m_aMIWeaponPrimary[12 + 1 + 1];	// 12 for the items, 1 for the highlight, and one for the info box.
	static CMenuItem m_aMIWeaponSecondary[12 + 1 + 1];	// 12 for the items, 1 for the highlight, and one for the info box.
	static CMenuItem m_aMIItem[6];						// 12 for the items.
	static CMenuItem m_aMIOptions[6];
	static CMenuScreen m_aMS[4];
	static CMenuMgr m_MenuMgr;

	//// Related to things in the frame area.
	//
	static CItemInst m_aIIDummy[2];
	static CFTexInst m_atexFrame[2];
	static FDrawVtx_t m_avtxFrame[24], m_avtxButton[6 * 5];
//ARG	static FTextAreaHandle_t m_hScreenTitle, m_ahButtonDesc[2];
	static FTextAreaHandle_t m_hScreenTitle, m_ahButtonDesc[4];	//ARG - should be 4 as well !!!
	static CFTexInst m_texControls;
	static u32 m_uFrameDrawFlags;
	//
	////

	static BOOL8 m_bMsgBoxActive;
	static BOOL8 m_bIsEnabled;
	static BOOL8 m_bWSEnabled;
	static BOOL8 m_bHUDEnabled;
//	static CMenuItem m_aMIYesNo[4];

	static CMenuItem m_MIBlueBox;

//	static f32 m_fEUKHighlightTimer;
//	static CMenuHighlight m_aEUKHighlight[20];
//	static CMenuScreen m_MSYesNo;
//	static CMenuMgr m_MMYesNo;
	static CFTexInst m_texHL;

	static CFTexInst m_BackgroundTexInst;
	static CFTexInst m_InvBackgroundTexInst;
	
	static CInventory *m_pInventory;

	static FTextAreaHandle_t m_hInfoName, m_hInfoDesc, m_hInfoUpgrade, m_hMissionTitle, m_hMissionText;
	static FViewport_t *m_pviewPerspective;
	static f32 m_fItemTheta;

	static PauseScreenState_e m_eState;

	static BOOL m_bShowMonitors;		// remembers state of script system text monitors
	static BOOL m_bIgnoreControlsNextFrame;
	static BOOL m_bQuitNextFrame;
	
	static BOOL m_bPauseAudio;
};

#endif
