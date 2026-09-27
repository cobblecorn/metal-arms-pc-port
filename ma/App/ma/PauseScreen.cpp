// PauseScreen.cpp: implementation of the CPauseScreen class.
//
// DFS - TODO: Figure out what to do with the pause screen for multiplayer.
// For now, always assume player 0.
//////////////////////////////////////////////////////////////////////

#include "PauseScreen.h"
#include "fviewport.h"
#include "bot.h"
#include "Item.h"
#include "ItemInst.h"
#include "MenuTypes.h"
#include "Hud2.h"
#include "ItemRepository.h"
#include "gamepad.h"
#include "gamesave.h"
#include "level.h"
#include "fscriptsystem.h"
#include "launcher.h"
#include "game.h"
#include "fresload.h"
#include "fclib.h"
#include "frenderer.h"
#include "wpr_system.h"
#include "msgbox.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif

#define _4_SCREEN_SETUP			FALSE// set to TRUE to have a 4 screen pause screen setup, otherwise only 2 (Options & Items)

#define _FLASHING_SHOULDER_ICON_SPEED (5.0f)

typedef enum {
	_MSGBOX_DATA_EXIT,
	_MSGBOX_DATA_RESTART,
	_MSGBOX_DATA_RESPAWN,
};

#define _LOSEDATA_MSGBOX		( "LoseDataWarning" )


// =============================================================================================================

// Utility functions.
FINLINE f32 P2JX(u32 uPixel) { return(((f32)(uPixel) - 320.0f) * (1.0f / 320.0f)); }
FINLINE f32 P2JY(u32 uPixel) { return(((f32)(uPixel) - 240.0f) * (-1.0f / 240.0f) * 0.75f); }
FINLINE f32 P2AX(u32 uPixel) { return((f32)(uPixel) * (1.0f / 640.0f)); }
FINLINE f32 P2AY(u32 uPixel) { return((f32)(uPixel) * (1.0f / 480.0f) * 0.75f); }

// =============================================================================================================

CFTexInst CMenuHighlight::m_tex;

// =============================================================================================================

CMenuHighlight::CMenuHighlight()
{
	m_bIsActive = FALSE;
}

// =============================================================================================================

CMenuHighlight::~CMenuHighlight()
{
}

// =============================================================================================================

void CMenuHighlight::Draw() const
{
	f32 fX1 = m_vecUL.x;
	f32 fX2 = fX1 + m_vecRect.x;
	f32 fY1 = m_vecUL.y;
	f32 fY2 = fY1 - m_vecRect.y;

	f32 fS1 = 0.75f;//0.75f + (1.0f / 256.0f);
	f32 fS2 = 0.847f;//1.0f;
	f32 fT1 = 0.14f;//0.25f + (1.0f / 256.0f);
	f32 fT2 = 0.25f;//0.5f;

	fdraw_SetTexture(&m_tex);

	FDrawVtx_t avtxList[4];
	u32 uCurVtx = 0;

	avtxList[uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = m_fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
	avtxList[uCurVtx].ST.Set(fS1, fT1);

	avtxList[++uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = m_fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
	avtxList[uCurVtx].ST.Set(fS2, fT1);

	avtxList[++uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = m_fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
	avtxList[uCurVtx].ST.Set(fS1, fT2);

	avtxList[++uCurVtx].ColorRGBA.White();
	avtxList[uCurVtx].ColorRGBA.fAlpha = m_fAlpha;
	avtxList[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
	avtxList[uCurVtx].ST.Set(fS2, fT2);

	++uCurVtx;

	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, avtxList, uCurVtx);
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

enum
{
	PSINPUT_START		=	1 << 0,

	PSINPUT_NONE		=	0
};

// =============================================================================================================

const static CFVec3 CPauseScreen_avecFrameUL[2] = { CFVec3(-1.0f, 0.75f, 0.0f), CFVec3(-1.0f, P2JY(480 - 80), 0.0f) };//{ CFVec3(-1.0f, 0.75f, 0.0f), CFVec3(-1.0f, -0.75f + 0.25f, 0.0f) };
// These are really the coordinates for the left half of the frame.
const static CFVec3 CPauseScreen_avecFrameBR[2] = { CFVec3(0.0f, P2JY(91), 0.0f), CFVec3(0.0f, -0.75f, 0.0f) };//CFVec3(1.0f, 0.75f - 0.25f, 0.0f), CFVec3(1.0f, -0.75f, 0.0f) };
const static char *CPauseScreen_apszFrameTex[2] = { "tfh2hudall2", "tfh2hudall2" };//"tfh2invbg01", "tfh2invbg03" };

//const static CFVec2 CPauseScreen_avecFrameSTUL[2] = { CFVec2(0.0f, 0.662f), CFVec2(0.0f, 0.841f) };
//const static CFVec2 CPauseScreen_avecFrameSTLR[2] = { CFVec2(0.623f, 0.835f), CFVec2(0.625f, 0.996f) };

const static CFVec2 CPauseScreen_avecFrameSTUL[2] = { CFVec2(-1.0f, 0.0f), CFVec2(-1.0f, 0.75f) };
const static CFVec2 CPauseScreen_avecFrameSTLR[2] = { CFVec2(2.0f, 0.70f), CFVec2(1.5f, 1.3f) };

static const f32 CPauseScreen_fButtonWidth = 0.087f;
static const f32 CPauseScreen_fButtonTextWidth = 0.68f;
static const f32 CPauseScreen_fButtonHeight = 0.077f;//0.070f;//0.075f;//0.065f;
static const u32 CPauseScreen_uNumButtonsDisplayed = 2;
static const CFVec3 CPauseScreen_vecButtonUL(-1.0f + 0.12f * 2.0f, -0.54f - 0.010f, 0.0f);
static const f32 CPauseScreen_fButtonSpacing = 0.034f;
const CFVec2 CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_COUNT] = {
	CFVec2( (0.0f/255.0f), 0.0f ),	// PAUSESCREEN_BUTTON_ST_A
	CFVec2( (33.0f/255.0f), 0.0f ),	// PAUSESCREEN_BUTTON_ST_B
	CFVec2( (135.0f/255.0f), 0.0f),	// PAUSESCREEN_BUTTON_ST_L
	CFVec2( (175.0f/255.0f), 0.0f),	// PAUSESCREEN_BUTTON_ST_R
	CFVec2( (66.0f/255.0f), 0.0f),	// PAUSESCREEN_BUTTON_ST_Y
	CFVec2( (109.0f/255.0f), 0.0f),	// PAUSESCREEN_BUTTON_ST_X
	CFVec2( (213.0f/255.0f), 0.0f),	// PAUSESCREEN_BUTTON_ST_START
};

const CFVec2 CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_COUNT] = {
	CFVec2( (33.0f/255.0f), 1.0f),	// PAUSESCREEN_BUTTON_ST_A
	CFVec2( (66.0f/255.0f), 1.0f),	// PAUSESCREEN_BUTTON_ST_B
	CFVec2( (175.0f/255.0f), 1.0f),	// PAUSESCREEN_BUTTON_ST_L
	CFVec2( (213.0f/255.0f), 1.0f),	// PAUSESCREEN_BUTTON_ST_R
	CFVec2( (109.0f/255.0f), 1.0f),	// PAUSESCREEN_BUTTON_ST_Y
	CFVec2( (135.0f/255.0f), 1.0f),	// PAUSESCREEN_BUTTON_ST_X
	CFVec2( (255.0f/255.0f), 1.0f),	// PAUSESCREEN_BUTTON_ST_START
};
static cwchar *CPauseScreen_apwszButtonText[2];		// [0]=Button A,  [1]=Button B

const static CFVec3 CPauseScreen_avecIconUL[2] = { CFVec3(-1.0f + 0.08f * 2.0f, P2JY(95), 0.0f), CFVec3(-1.0f + 0.08f * 2.0f, P2JY(95), 0.0f) };
const static CFVec3 CPauseScreen_avecIconRectWeapon[2] = { CFVec3(0.288f, 0.2f, 0.0f), CFVec3(0.288f, 0.2f, 0.0f) };
const static f32 CPauseScreen_afIconSpacingWeaponX[2] = { 0.065f, 0.065f };
const static f32 CPauseScreen_afIconSpacingWeaponY[2] = { 0.046f, 0.046f };

const static CFVec3 CPauseScreen_vecItemUL = CFVec3(-1.0f + 0.08f * 2.0f, P2JY(95), 0.0f);
const static CFVec3 CPauseScreen_vecItemRect = CFVec3(0.288f, 0.2f, 0.0f);
const static f32 CPauseScreen_fItemSpacingX = 0.065f;//0.12f;//0.05f;
const static f32 CPauseScreen_fItemSpacingY = 0.046f;

const static CFVec3 CPauseScreen_vecOptionsUL = CFVec3(-0.42f, 0.32f, 0.0f);
const static CFVec3 CPauseScreen_vecOptionsRect = CFVec3(0.835f, 0.07f/*0.08f*/, 0.0f);
const static f32 CPauseScreen_fOptionsSpacingY = 0.04f;
static cwchar *CPauseScreen_apwszOptions[6];

const static CFVec3 CPauseScreen_avecYesNoUL[2] = { CFVec3(-0.45f, -0.07f, 0.0f), CFVec3(-0.05f, -0.07f, 0.0f) };
const static CFVec3 CPauseScreen_avecYesNoRect[2] = { CFVec3(0.5f, 0.08f, 0.0f), CFVec3(0.5f, 0.08f, 0.0f) };

const static CFVec3 CPauseScreen_vecYesNoTitleUL = CFVec3(-0.3f, 0.2f, 0.0f);
const static CFVec3 CPauseScreen_vecYesNoTitleRect = CFVec3(0.6f, 0.08f, 0.0f);

static const CFVec3 CPauseScreen_vecYesNoBoxUL(-0.6f, 0.25f, 0.0f);
static const CFVec3 CPauseScreen_vecYesNoRect(1.2f, 0.6f, 0.0f);

static const f32 CPauseScreen_fInfoNameLE = P2AX(388);
static const f32 CPauseScreen_fInfoNameRE = P2AX(531);
static const f32 CPauseScreen_fInfoNameTE = P2AY(277);
static const f32 CPauseScreen_fInfoNameBE = P2AY(297);

static const f32 CPauseScreen_fInfoDescLE = P2AX(388);
static const f32 CPauseScreen_fInfoDescRE = P2AX(585);//P2AX(579);
static const f32 CPauseScreen_fInfoDescTE = P2AY(301);
static const f32 CPauseScreen_fInfoDescBE = P2AY(366);

static const CFVec3 CPauseScreen_vecInfoBoxUL(P2JX(382), P2JY(95), 0.0f);//, 0.75f - (0.08f * 1.5f) - 0.15f, 0.0f);
static const CFVec3 CPauseScreen_vecInfoBoxLR(P2JX(591), P2JY(396), 0.0f);//1.0f - 0.08f * 2.0f, -0.75f + (0.08f * 1.5f) + 0.15f, 0.0f);
static const f32 CPauseScreen_fInfoBoxMeshLE = P2JX(391);
static const f32 CPauseScreen_fInfoBoxMeshRE = P2JX(565);
static const f32 CPauseScreen_fInfoBoxMeshTE = P2JY(118);
static const f32 CPauseScreen_fInfoBoxMeshBE = P2JY(238);
static const f32 CPauseScreen_fItemOmegaYaw = 2.0f;

static const u32 CPauseScreen_auScreenItemWidth[3] = { 3, 3, 3 };
static const u32 CPauseScreen_auScreenItemHeight[3] = { 4, 4, 2 };

static cwchar *CPauseScreen_apwszScreenTitle[4];
static cchar *CPauseScreen_apszNullItemName[3] = { "Null Primary", "Null Secondary", "Null Item" };

// Derived constants:
static const CFVec3 CPauseScreen_vecButtonRegionRect(CPauseScreen_fButtonWidth + CPauseScreen_fButtonTextWidth, CPauseScreen_fButtonHeight, 0.0f);
static const u32 CPauseScreen_auSelectedIdx[2] = { CPauseScreen_auScreenItemHeight[0] * CPauseScreen_auScreenItemWidth[0], CPauseScreen_auScreenItemHeight[1] * CPauseScreen_auScreenItemWidth[1] };

static f32 _fUnitFlash;
static Wpr_DataTypes_Screens_e _nActiveWrapperScreen;	// WPR_DATATYPES_SCREENS_NONE if none

// =============================================================================================================

u32 CPauseScreen::m_uButtons, CPauseScreen::m_uButtonsLatched;
CMenuItem CPauseScreen::m_aMIWeaponPrimary[12 + 1 + 1];
CMenuItem CPauseScreen::m_aMIWeaponSecondary[12 + 1 + 1];
CMenuItem CPauseScreen::m_aMIItem[6];
CMenuItem CPauseScreen::m_aMIOptions[6];
CMenuScreen CPauseScreen::m_aMS[4];
CMenuMgr CPauseScreen::m_MenuMgr;
FViewport_t *CPauseScreen::m_pviewOrtho3d;
CItemInst CPauseScreen::m_aoNullSlot[3];
CInventory *CPauseScreen::m_pInventory;
PauseScreenState_e CPauseScreen::m_eState;
FTextAreaHandle_t CPauseScreen::m_hInfoName, CPauseScreen::m_hInfoDesc, CPauseScreen::m_hInfoUpgrade, CPauseScreen::m_hMissionTitle, CPauseScreen::m_hMissionText;

FViewport_t *CPauseScreen::m_pviewPerspective;
f32 CPauseScreen::m_fItemTheta;
CMenuItem CPauseScreen::m_MIBlueBox;
CFTexInst CPauseScreen::m_texHL;

CFTexInst CPauseScreen::m_atexFrame[2];
FDrawVtx_t CPauseScreen::m_avtxFrame[24], CPauseScreen::m_avtxButton[6 * 5];
FTextAreaHandle_t CPauseScreen::m_hScreenTitle, CPauseScreen::m_ahButtonDesc[4];	//ARG - should be 4 as well !!!
CFTexInst CPauseScreen::m_texControls;
u32 CPauseScreen::m_uFrameDrawFlags;

CItemInst CPauseScreen::m_aIIDummy[2];

CFTexInst CPauseScreen::m_BackgroundTexInst;

BOOL8 CPauseScreen::m_bIsEnabled = FALSE;
BOOL8 CPauseScreen::m_bWSEnabled = FALSE;
BOOL8 CPauseScreen::m_bHUDEnabled = FALSE;
BOOL8 CPauseScreen::m_bMsgBoxActive	= FALSE;
BOOL CPauseScreen::m_bShowMonitors;
BOOL CPauseScreen::m_bIgnoreControlsNextFrame = FALSE;
BOOL CPauseScreen::m_bQuitNextFrame = FALSE;

BOOL CPauseScreen::m_bPauseAudio=TRUE;
#if defined(MA_PC_INPUT)
CFTexInst CPauseScreen::m_MousePointerTex;
#endif

// =============================================================================================================

BOOL CPauseScreen::InitSystem()
{
	m_bIsEnabled = TRUE;
	SetWSEnable(TRUE);

	//{ "Restart Level", "Sound Options", "Controller Map", "Advanced Settings", "Quit Game" };
	CPauseScreen_apwszOptions[0] = Game_apwszPhrases[ GAMEPHRASE_ADVANCED_CONTROLLER_SETUP ];
	CPauseScreen_apwszOptions[1] = Game_apwszPhrases[ GAMEPHRASE_SOUND_OPTIONS ];
	CPauseScreen_apwszOptions[2] = Game_apwszPhrases[ GAMEPHRASE_EDIT_CONTROLLER ];
	CPauseScreen_apwszOptions[3] = Game_apwszPhrases[ GAMEPHRASE_RESPAWN ];
	CPauseScreen_apwszOptions[4] = Game_apwszPhrases[ GAMEPHRASE_RESTART_LEVEL ];
	CPauseScreen_apwszOptions[5] = Game_apwszPhrases[ GAMEPHRASE_QUIT_GAME ];
		
	//{ "Primary Equipment", "Secondary Equipment", "Information", "Options" };
	CPauseScreen_apwszScreenTitle[0] = Game_apwszPhrases[ GAMEPHRASE_PRIMARY_EQUIPMENT ];
	CPauseScreen_apwszScreenTitle[1] = Game_apwszPhrases[ GAMEPHRASE_SECONDARY_EQUIPMENT ];
	CPauseScreen_apwszScreenTitle[2] = Game_apwszPhrases[ GAMEPHRASE_INFORMATION ];
	CPauseScreen_apwszScreenTitle[3] = Game_apwszPhrases[ GAMEPHRASE_OPTIONS ];

	// Create all of the CMenuItem objects, initialize their positions and interconnections.
	// Then create all of the CMenuScreen objects, and initialize their CMenuItems and interconnections.
	// Then create the one CMenuMgr and initialize it to have the above CMenuScreens.

	////
	//
	CMenuHighlight::m_tex.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2hudall1")) );
	if(CMenuHighlight::m_tex.GetTexDef() == NULL)
	{
		DEVPRINTF("CPauseScreen::InitSystem() : Could not load menu highlight texture.\n");
	}
	//
	////


	//// Initialize positions and interconnections for the menu items.
	//
	u32 uX, uY, uCurMIIdx;
	CMenuItem *pCurMI;

	// For the blue info box.
	// TODO: Change this texture and its STs.
	FTexDef_t *ptexBG = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh1hudall3"/*"tfh2invbg02"*/));
	if(ptexBG == NULL)
	{
		DEVPRINTF("CPauseScreen::InitSystem() : Could not load background texture.\n");
	}
	m_BackgroundTexInst.SetTexDef( ptexBG );

	// CHANGED:
	m_MIBlueBox.m_vecBorderUL = CPauseScreen_vecInfoBoxUL;
	m_MIBlueBox.m_avecImageOfs[0].Set(0.0f, 0.0f, 0.0f);// = CPauseScreen_vecInfoBoxUL;
	f32 fWidth = CPauseScreen_vecInfoBoxLR.x - CPauseScreen_vecInfoBoxUL.x;
	f32 fHeight = CPauseScreen_vecInfoBoxUL.y - CPauseScreen_vecInfoBoxLR.y;
	m_MIBlueBox.m_vecBorderRect.x = fWidth;
	m_MIBlueBox.m_vecBorderRect.y = fHeight;
	m_MIBlueBox.m_avecImageRect[0].x = fWidth;
	m_MIBlueBox.m_avecImageRect[0].y = fHeight;
	m_MIBlueBox.SetGraphic(0, &m_BackgroundTexInst, CFVec2(0.0f, 0.0f), CFVec2(0.410f, 0.587f)/*CFVec2(fWidth * CPauseScreen_fInfoBoxSTScaleFactor, fHeight * CPauseScreen_fInfoBoxSTScaleFactor)*/);
	m_MIBlueBox.Init();

	u32 uCurWidth, uCurHeight;

	// For the primary weapon screen.
	uCurWidth = CPauseScreen_auScreenItemWidth[0];
	uCurHeight = CPauseScreen_auScreenItemHeight[0];
	for(uY = 0; uY < uCurHeight; ++uY)
	{
		for(uX = 0; uX < uCurWidth; ++uX)
		{
			// CHANGED:
			uCurMIIdx = (uY * uCurWidth) + uX;
			m_aMIWeaponPrimary[uCurMIIdx].m_apNext[MIDIR_UP] = (uY == 0) ? NULL : &(m_aMIWeaponPrimary[uCurMIIdx - uCurWidth]);
			m_aMIWeaponPrimary[uCurMIIdx].m_apNext[MIDIR_DOWN] = (uY == (uCurHeight - 1)) ? NULL : &(m_aMIWeaponPrimary[uCurMIIdx + uCurWidth]);
			m_aMIWeaponPrimary[uCurMIIdx].m_apNext[MIDIR_LEFT] = (uX == 0) ? NULL : &(m_aMIWeaponPrimary[uCurMIIdx - 1]);
			m_aMIWeaponPrimary[uCurMIIdx].m_apNext[MIDIR_RIGHT] = (uX == (uCurWidth - 1)) ? NULL : &(m_aMIWeaponPrimary[uCurMIIdx + 1]);
			// For the item border.
			m_aMIWeaponPrimary[uCurMIIdx].m_vecBorderUL = CPauseScreen_avecIconUL[0];
			m_aMIWeaponPrimary[uCurMIIdx].m_vecBorderUL.x += (f32)(uX) * (CPauseScreen_avecIconRectWeapon[0].x + CPauseScreen_afIconSpacingWeaponX[0]);
			m_aMIWeaponPrimary[uCurMIIdx].m_vecBorderUL.y -= (f32)(uY) * (CPauseScreen_avecIconRectWeapon[0].y + CPauseScreen_afIconSpacingWeaponY[0]);
			m_aMIWeaponPrimary[uCurMIIdx].m_vecBorderRect = CPauseScreen_avecIconRectWeapon[0];
			// For the image rectangle.
			m_aMIWeaponPrimary[uCurMIIdx].m_avecImageOfs[0].Set(0.0f, 0.0f, 0.0f);// = CPauseScreen_avecIconUL[0];
			m_aMIWeaponPrimary[uCurMIIdx].m_avecImageRect[0] = CPauseScreen_avecIconRectWeapon[0];

			m_aMIWeaponPrimary[uCurMIIdx].Init();

			m_aMIWeaponPrimary[uCurMIIdx].m_pfcnCallback = WeaponCallback;
		}
	}

	pCurMI = &(m_aMIWeaponPrimary[CPauseScreen_auSelectedIdx[0]]);
	FTexDef_t *ptexSelected = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2hudall1"));
	if(ptexSelected == NULL)
	{
		DEVPRINTF("CPauseScreen::InitSystem() : Could not load selected weapon texture.\n");
	}
	else
	{
//		pCurMI->SetGraphic(0, ptexSelected, CFVec2(0.75f, 0.5f), CFVec2(1.0f, 0.75f), CFVec3(0.0f, 0.0f, 0.0f), CPauseScreen_avecIconRectWeapon[0]);
//		pCurMI->SetGraphic(0, ptexSelected, CFVec2(0.120f, 0.250f), CFVec2(0.496f, 0.500f), CFVec3(0.0f, 0.0f, 0.0f), CPauseScreen_avecIconRectWeapon[0]);
		m_aIIDummy[0].m_pItemData = CItemRepository::RetrieveEntry("SPrim", NULL);
		FASSERT(m_aIIDummy[0].m_pItemData != NULL);
//		m_aIIDummy[0].m_uCurAmmo = 0;
		// JUSTIN: This doesn't need to be here.
		m_aIIDummy[0].m_nReserveAmmo = CItemInst_nNoAmmo;
		pCurMI->Init();
		pCurMI->InitWithItemInst(&m_aIIDummy[0]);
		pCurMI->m_avecST1[1].y -= 0.25f;
		pCurMI->m_avecST2[1].y -= 0.25f;
		pCurMI->m_pTex[2] = NULL;
	}

	// For the secondary weapon screen.
	uCurWidth = CPauseScreen_auScreenItemWidth[1];
	uCurHeight = CPauseScreen_auScreenItemHeight[1];
	for(uY = 0; uY < uCurHeight; ++uY)
	{
		for(uX = 0; uX < uCurWidth; ++uX)
		{
			uCurMIIdx = (uY * uCurWidth) + uX;
			m_aMIWeaponSecondary[uCurMIIdx].m_apNext[MIDIR_UP] = (uY == 0) ? NULL : &(m_aMIWeaponSecondary[uCurMIIdx - uCurWidth]);
			m_aMIWeaponSecondary[uCurMIIdx].m_apNext[MIDIR_DOWN] = (uY == (uCurHeight - 1)) ? NULL : &(m_aMIWeaponSecondary[uCurMIIdx + uCurWidth]);
			m_aMIWeaponSecondary[uCurMIIdx].m_apNext[MIDIR_LEFT] = (uX == 0) ? NULL : &(m_aMIWeaponSecondary[uCurMIIdx - 1]);
			m_aMIWeaponSecondary[uCurMIIdx].m_apNext[MIDIR_RIGHT] = (uX == (uCurWidth - 1)) ? NULL : &(m_aMIWeaponSecondary[uCurMIIdx + 1]);
			// CHANGED:
			m_aMIWeaponSecondary[uCurMIIdx].m_vecBorderUL = CPauseScreen_avecIconUL[1];
			m_aMIWeaponSecondary[uCurMIIdx].m_vecBorderUL.x += (f32)(uX) * (CPauseScreen_avecIconRectWeapon[1].x + CPauseScreen_afIconSpacingWeaponX[1]);
			m_aMIWeaponSecondary[uCurMIIdx].m_vecBorderUL.y -= (f32)(uY) * (CPauseScreen_avecIconRectWeapon[1].y + CPauseScreen_afIconSpacingWeaponY[1]);
			m_aMIWeaponSecondary[uCurMIIdx].m_vecBorderRect = CPauseScreen_avecIconRectWeapon[1];
			// CHANGED:
			m_aMIWeaponSecondary[uCurMIIdx].m_avecImageOfs[0].Set(0.0f, 0.0f, 0.0f);// = CPauseScreen_avecIconUL[1];
			m_aMIWeaponSecondary[uCurMIIdx].m_avecImageRect[0] = CPauseScreen_avecIconRectWeapon[1];

			m_aMIWeaponSecondary[uCurMIIdx].Init();

			m_aMIWeaponSecondary[uCurMIIdx].m_pfcnCallback = WeaponCallback;
		}
	}
	// TODO: We should have two textures here.
	pCurMI = &(m_aMIWeaponSecondary[CPauseScreen_auSelectedIdx[1]]);
	if(ptexSelected != NULL)
	{
//		pCurMI->SetGraphic(0, ptexSelected, CFVec2(0.75f, 0.5f), CFVec2(1.0f, 0.75f), CFVec3(0.0f, 0.0f, 0.0f), CPauseScreen_avecIconRectWeapon[1]);
//		pCurMI->SetGraphic(0, ptexSelected, CFVec2(0.492f, 0.250f), CFVec2(0.113f, 0.492f), CFVec3(0.0f, 0.0f, 0.0f), CPauseScreen_avecIconRectWeapon[1]);
		m_aIIDummy[1].m_pItemData = CItemRepository::RetrieveEntry("SSeco", NULL);
		FASSERT(m_aIIDummy[1].m_pItemData != NULL);
//		m_aIIDummy[1].m_uCurAmmo = 0;
		// JUSTIN: This doesn't need to be here.
		m_aIIDummy[1].m_nReserveAmmo = CItemInst_nNoAmmo;
		pCurMI->Init();
		pCurMI->InitWithItemInst(&m_aIIDummy[1]);
		pCurMI->m_avecST1[1].y -= 0.25f;
		pCurMI->m_avecST2[1].y -= 0.25f;
		pCurMI->m_pTex[2] = NULL;
	}

	//// For the items screen.
	//
	uCurWidth = CPauseScreen_auScreenItemWidth[2];
	uCurHeight = CPauseScreen_auScreenItemHeight[2];
	for(uY = 0; uY < uCurHeight; ++uY)
	{
		for(uX = 0; uX < uCurWidth; ++uX)
		{
			uCurMIIdx = (uY * uCurWidth) + uX;
			m_aMIItem[uCurMIIdx].m_apNext[MIDIR_UP] = (uY == 0) ? NULL : &(m_aMIItem[uCurMIIdx - uCurWidth]);
			m_aMIItem[uCurMIIdx].m_apNext[MIDIR_DOWN] = (uY == (uCurHeight - 1)) ? NULL : &(m_aMIItem[uCurMIIdx + uCurWidth]);
			m_aMIItem[uCurMIIdx].m_apNext[MIDIR_LEFT] = (uX == 0) ? NULL : &(m_aMIItem[uCurMIIdx - 1]);
			m_aMIItem[uCurMIIdx].m_apNext[MIDIR_RIGHT] = (uX == (uCurWidth - 1)) ? NULL : &(m_aMIItem[uCurMIIdx + 1]);
			// CHANGED:
			m_aMIItem[uCurMIIdx].m_vecBorderUL = CPauseScreen_vecItemUL;
			m_aMIItem[uCurMIIdx].m_vecBorderUL.x += (f32)(uX) * (CPauseScreen_vecItemRect.x + CPauseScreen_fItemSpacingX);
			m_aMIItem[uCurMIIdx].m_vecBorderUL.y -= (f32)(uY) * (CPauseScreen_vecItemRect.y + CPauseScreen_fItemSpacingY);
			m_aMIItem[uCurMIIdx].m_vecBorderRect = CPauseScreen_vecItemRect;
			// CHANGED:
			m_aMIItem[uCurMIIdx].m_avecImageOfs[0].Set(0.0f, 0.0f, 0.0f);// = CPauseScreen_vecItemUL;
			m_aMIItem[uCurMIIdx].m_avecImageRect[0] = CPauseScreen_vecItemRect;

			m_aMIItem[uCurMIIdx].Init();

			m_aMIItem[uCurMIIdx].m_pfcnCallback = CancelOnlyCallback;
		}
	}
	//
	////

	//// For the options screen.
	//
	FTextArea_t oTextArea;
	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.oColorForeground.Set( 0.6f, 0.6f, 0.6f, 1.0f );
	oTextArea.oColorBackground.Transparent();
	oTextArea.ohFont = '1';
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	for(uCurMIIdx = 0; uCurMIIdx < 6; ++uCurMIIdx)
	{
		m_aMIOptions[uCurMIIdx].m_apNext[MIDIR_UP] = (uCurMIIdx == 0) ? NULL : &(m_aMIOptions[uCurMIIdx - 1]);
		m_aMIOptions[uCurMIIdx].m_apNext[MIDIR_DOWN] = (uCurMIIdx == 5) ? NULL : &(m_aMIOptions[uCurMIIdx + 1]);
		m_aMIOptions[uCurMIIdx].m_apNext[MIDIR_LEFT] = NULL;
		m_aMIOptions[uCurMIIdx].m_apNext[MIDIR_RIGHT] = NULL;

		m_aMIOptions[uCurMIIdx].m_vecBorderUL = CPauseScreen_vecOptionsUL;
		m_aMIOptions[uCurMIIdx].m_vecBorderUL.y -= (f32)(uCurMIIdx) * (CPauseScreen_fOptionsSpacingY + CPauseScreen_vecOptionsRect.y);
		m_aMIOptions[uCurMIIdx].m_vecBorderRect = CPauseScreen_vecOptionsRect;
		// TODO: I don't think this is necessary.
		m_aMIOptions[uCurMIIdx].m_avecImageOfs[0].Set(0.0f, 0.0f, 0.0f);
		m_aMIOptions[uCurMIIdx].m_avecImageRect[0] = CPauseScreen_vecOptionsRect;

		m_aMIOptions[uCurMIIdx].m_avecTextOfs[0].Set(0.0f, 0.0f);
		m_aMIOptions[uCurMIIdx].m_avecTextRect[0].Set(CPauseScreen_vecOptionsRect.x, CPauseScreen_vecOptionsRect.y);
		m_aMIOptions[uCurMIIdx].SetText(0, CPauseScreen_apwszOptions[uCurMIIdx]);
		m_aMIOptions[uCurMIIdx].m_ahText[0] = ftext_Create(&oTextArea);

		m_aMIOptions[uCurMIIdx].Init();

		m_aMIOptions[uCurMIIdx].m_pfcnCallback = OptionScreenCallback;//CancelOnlyCallback;

	}
	//
	////

#if 0
	FTexDef_t  *ptexTemp = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tfh2invbg04"));
	if(ptexTemp == NULL)
	{
		DEVPRINTF("CPauseSystem::InitSystem() : Could not load background texture.\n");
	}
	else
	{
		m_InvBackgroundTexInst.SetTexDef( ptexTemp );
	}
	
	if ( m_InvBackgroundTexInst.GetTexDef() != NULL )
	{
		pCurMI->SetGraphic(0, &m_InvBackgroundTexInst, CFVec2(0.0f, 0.0f), CFVec2(1.0f, 1.0f));
	}
	//
	////
#endif

#if _4_SCREEN_SETUP// the old 4 screen pause screen

	//// For the main screens.
	//
	m_aMS[0].Reset();
	for(uCurMIIdx = 0; uCurMIIdx < 14; ++uCurMIIdx)
		m_aMS[0].AddMI(&(m_aMIWeaponPrimary[uCurMIIdx]));
	m_aMS[0].AddMI(&m_MIBlueBox);
	m_aMS[0].m_apNext[0] = &(m_aMS[3]);
	m_aMS[0].m_apNext[1] = &(m_aMS[1]);
	m_aMS[0].SetTitleText(CPauseScreen_apwszScreenTitle[0]);

	m_aMS[1].Reset();
	for(uCurMIIdx = 0; uCurMIIdx < 14; ++uCurMIIdx)
		m_aMS[1].AddMI(&(m_aMIWeaponSecondary[uCurMIIdx]));
	m_aMS[1].AddMI(&m_MIBlueBox);
	m_aMS[1].m_apNext[0] = &(m_aMS[0]);
	m_aMS[1].m_apNext[1] = &(m_aMS[2]);
	m_aMS[1].SetTitleText(CPauseScreen_apwszScreenTitle[1]);

	m_aMS[2].Reset();
	for(uCurMIIdx = 0; uCurMIIdx < 6; ++uCurMIIdx)
		m_aMS[2].AddMI(&(m_aMIItem[uCurMIIdx]));
	m_aMS[2].AddMI(&m_MIBlueBox);
	m_aMS[2].m_apNext[0] = &(m_aMS[1]);
	m_aMS[2].m_apNext[1] = &(m_aMS[3]);
	m_aMS[2].SetTitleText(CPauseScreen_apwszScreenTitle[2]);

	m_aMS[3].Reset();
	for(uCurMIIdx = 0; uCurMIIdx < 6; ++uCurMIIdx)
		m_aMS[3].AddMI(&(m_aMIOptions[uCurMIIdx]));
	m_aMS[3].m_apNext[0] = &(m_aMS[2]);
	m_aMS[3].m_apNext[1] = &(m_aMS[0]);
	m_aMS[3].SetTitleText(CPauseScreen_apwszScreenTitle[3]);
	//
	////
#else
	// For the main screens.

	// options
	m_aMS[0].Reset();
	for( uCurMIIdx = 0; uCurMIIdx < 6; ++uCurMIIdx ) {
		m_aMS[0].AddMI( &m_aMIOptions[uCurMIIdx] );
	}
	m_aMS[0].m_apNext[0] = &m_aMS[1];// left
	m_aMS[0].m_apNext[1] = &m_aMS[1];// right
	m_aMS[0].SetTitleText( CPauseScreen_apwszScreenTitle[3] );

	// item
	m_aMS[1].Reset();
	for( uCurMIIdx = 0; uCurMIIdx < 6; ++uCurMIIdx ) {
		m_aMS[1].AddMI( &m_aMIItem[uCurMIIdx] );
	}
	m_aMS[1].AddMI( &m_MIBlueBox );
	m_aMS[1].m_apNext[0] = &m_aMS[0];// left
	m_aMS[1].m_apNext[1] = &m_aMS[0];// right
	m_aMS[1].SetTitleText( CPauseScreen_apwszScreenTitle[2] );	
#endif

#if 0
	//// For the yes/no screen.
	//
	m_MSYesNo.Reset();
	for (uCurMIIdx = 0; uCurMIIdx < 4; ++uCurMIIdx)
		m_MSYesNo.AddMI(&(m_aMIYesNo[uCurMIIdx]));
	//
	////
#endif

	////
	//
	m_MenuMgr.SetCurrent(&(m_aMS[0]));
	//
	////

	////
	//
//	m_MMYesNo.SetCurrent(&m_MSYesNo);
	//
	////

	u32 uCurWeap;
	for(uCurWeap = 0; uCurWeap < 3; ++uCurWeap)
	{
		m_aoNullSlot[uCurWeap].m_pItemData = CItemRepository::RetrieveEntry(CPauseScreen_apszNullItemName[uCurWeap], NULL);
		if(m_aoNullSlot[uCurWeap].m_pItemData == NULL)
		{
			DEVPRINTF("CPauseScreen::InitSystem : Could not load item data for'%s'.\n", CPauseScreen_apszNullItemName[uCurWeap]);
		}
		else
		{
//			m_aoNullSlot[uCurWeap].m_uCurAmmo = 0;
			m_aoNullSlot[uCurWeap].m_nReserveAmmo  = CItemInst_nNoAmmo;
		}
	}

	SetupFrame();

	m_pviewOrtho3d = fviewport_Create();
	if(m_pviewOrtho3d == NULL)
	{
		DEVPRINTF("CHud2::InitSystem : Could not create ortho 3d viewport.\n");
		return(FALSE);
	}
	fviewport_InitOrtho3D(m_pviewOrtho3d, 0.1f, 1000.0f);

	//// Create an area for our info text box.
	//
	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 2.0f;
	oTextArea.ohFont = '3';
	oTextArea.fUpperLeftY = CPauseScreen_fInfoNameTE;
	oTextArea.fLowerRightY = CPauseScreen_fInfoNameBE;
	oTextArea.oColorBackground.OpaqueRed();
	oTextArea.fBorderThicknessX = 0.0f;
	oTextArea.fBorderThicknessY = 0.0f;
	oTextArea.oColorBorder.TransparentBlack();
	oTextArea.bVisible = FALSE;
	oTextArea.fUpperLeftX = CPauseScreen_fInfoNameLE;
	oTextArea.fLowerRightX = CPauseScreen_fInfoNameRE;
	m_hInfoName = ftext_Create(&oTextArea);

	oTextArea.ohFont = '8'; 
	oTextArea.fNumberOfLines = 6.0f; 
	//oTextArea.oColorForeground.Set( 0.75f, 0.75f, 0.75f, 1.0f ); //set to a little darker white
	oTextArea.fUpperLeftX = CPauseScreen_fInfoDescLE;
	oTextArea.fUpperLeftY = CPauseScreen_fInfoDescTE;
	oTextArea.fLowerRightX = CPauseScreen_fInfoDescRE;
	oTextArea.fLowerRightY = CPauseScreen_fInfoDescBE;
	m_hInfoDesc = ftext_Create(&oTextArea);

	//oTextArea.oColorForeground.Set( 0.8f, 0.8f, 0.8f, 1.0f ); //set to default white
	oTextArea.fUpperLeftX += 0.05f;
	oTextArea.fUpperLeftY += 0.09f;		// 0.1f
	oTextArea.fLowerRightX -= 0.0f;
	oTextArea.fLowerRightY += 0.12f;	// 0.08f
	m_hInfoUpgrade = ftext_Create(&oTextArea);
	//
	////

	//// Create and initialize the perspective viewport for the weapon info box.
	//
	u32 uMeshLE_PC, uMeshWidth_PC, uMeshTE_PC, uMeshHeight_PC;
	m_pviewPerspective = fviewport_Create();
	if(m_pviewPerspective == NULL)
	{
		DEVPRINTF("CPauseScreen::InitSystem : Could not create perspective viewport.\n");
		return(FALSE);
	}

	uMeshLE_PC = (u32)(((CPauseScreen_fInfoBoxMeshLE + 1.0f) * 0.5f) * (f32)(FVid_Mode.nPixelsAcross));
	uMeshWidth_PC = (u32)(((CPauseScreen_fInfoBoxMeshRE - CPauseScreen_fInfoBoxMeshLE) * 0.5f) * (f32)(FVid_Mode.nPixelsAcross));
	uMeshTE_PC = (u32)(((-CPauseScreen_fInfoBoxMeshTE + 0.75f) * (2.0f / 3.0f)) * (f32)(FVid_Mode.nPixelsDown));
	uMeshHeight_PC = (u32)(((CPauseScreen_fInfoBoxMeshTE - CPauseScreen_fInfoBoxMeshBE) * (2.0f / 3.0f)) * (f32)(FVid_Mode.nPixelsDown));
	fviewport_InitPersp(m_pviewPerspective, /*FMATH_QUARTER_PI*/FMATH_PI * (1.0f / 6.0f), 0.1f, 1000.0f, uMeshLE_PC, uMeshTE_PC, uMeshWidth_PC, uMeshHeight_PC);
	//
	////

	//// Initialize the text area for the title.
	//
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	oTextArea.fUpperLeftX = 0.2f;
	oTextArea.fLowerRightX = 0.8f;
	oTextArea.fUpperLeftY = 0.08f * 0.75f;
	oTextArea.fLowerRightY = 0.13f * 0.75f;
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.ohFont = '3';
	m_hScreenTitle = ftext_Create(&oTextArea);
	//
	////

	//// Init the button icon textures and button text regions.
	//
	m_texControls.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, CPauseScreen_pszControlsTex)) );
	if( m_texControls.GetTexDef() == NULL ) {
		DEVPRINTF("CPauseScreen::InitSystem() : Could not load button texture.\n");
	}

	m_texControls.ClearFlag( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T | CFTexInst::FLAG_WRAP_U );

	// Mission title:
	ftext_SetToDefaults(&oTextArea);
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.ohFont = '3';
	oTextArea.fUpperLeftY = P2AY(282);
	oTextArea.fLowerRightY = P2AY(307);
	oTextArea.fUpperLeftX = P2AX(50);//P2AX(388);
	oTextArea.fLowerRightX = P2AX(350);//P2AX(531);
	oTextArea.fBorderThicknessX = 0.01f;
	oTextArea.fBorderThicknessY = 0.01f;
	oTextArea.oColorBackground.Set( 0.05f, 0.05f, 0.3f, 1.0f );
	oTextArea.oColorBorder = oTextArea.oColorBackground;
	m_hMissionTitle = ftext_Create(&oTextArea);

	// Mission text:
	oTextArea.ohFont = '8'; 
	oTextArea.fNumberOfLines = 4.0f; 
	oTextArea.fUpperLeftX = P2AX(50);//P2AX(388);
	oTextArea.fUpperLeftY = P2AY(301);
	oTextArea.fLowerRightX = P2AX(350);//P2AX(585);
	oTextArea.fLowerRightY = P2AY(378);
	oTextArea.fBorderThicknessX = 0.02f;
	oTextArea.fBorderThicknessY = 0.02f;
	m_hMissionText = ftext_Create(&oTextArea);


	ftext_SetToDefaults(&oTextArea);
	oTextArea.ohFont = '3';
	oTextArea.fNumberOfLines = 1.0f;

	u32 uCurButtonNum, uCurVtx = 0;
	f32 fX1, fX2, fY1, fY2;
	fX1 = CPauseScreen_vecButtonUL.x;
	fX2 = fX1 + CPauseScreen_fButtonWidth;
	fY1 = CPauseScreen_vecButtonUL.y;
	fY2 = fY1 - CPauseScreen_fButtonHeight;

	f32 fS1, fS2, fT1, fT2;

	for(uCurButtonNum = 0; uCurButtonNum < CPauseScreen_uNumButtonsDisplayed; ++uCurButtonNum)
	{
		fS1 = CPauseScreen_avecButtonST1[uCurButtonNum].x;
		fT1 = CPauseScreen_avecButtonST1[uCurButtonNum].y;
		fS2 = CPauseScreen_avecButtonST2[uCurButtonNum].x;
		fT2 = CPauseScreen_avecButtonST2[uCurButtonNum].y;

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
		m_avtxButton[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
		m_avtxButton[uCurVtx].ST.Set(fS2, fT1);

		m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
		m_avtxButton[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
		m_avtxButton[uCurVtx].ST.Set(fS1, fT2);

		m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
		m_avtxButton[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
		m_avtxButton[uCurVtx].ST.Set(fS2, fT2);

		++uCurVtx;

		oTextArea.fUpperLeftX = (fX1 + CPauseScreen_fButtonWidth + 1.02f) * 0.5f;
		oTextArea.fLowerRightX = oTextArea.fUpperLeftX + CPauseScreen_fButtonTextWidth * 0.5f;
		oTextArea.fUpperLeftY = (0.75f - fY1 + 0.015f) * 0.5f;
		oTextArea.fLowerRightY = (0.75f - fY2 - 0.02f + 0.015f) * 0.5f;

		m_ahButtonDesc[uCurButtonNum] = ftext_Create(&oTextArea);

		fX1 += CPauseScreen_vecButtonRegionRect.x;
		fX1 += CPauseScreen_fButtonSpacing;
		fX2 += CPauseScreen_vecButtonRegionRect.x;
		fX2 += CPauseScreen_fButtonSpacing;
	}

	fX1 = -1.0f + 0.095f * 2.0f;
	fX2 = fX1 + 0.14f;//CPauseScreen_fButtonWidth;
	fY1 = 0.75f - 0.07f * 1.5f;
	fY2 = fY1 - 0.12f;//CPauseScreen_fButtonHeight;

	for(uCurButtonNum = 2; uCurButtonNum < 4; ++uCurButtonNum)
	{
		fS1 = CPauseScreen_avecButtonST1[uCurButtonNum].x;
		fT1 = CPauseScreen_avecButtonST1[uCurButtonNum].y;
		fS2 = CPauseScreen_avecButtonST2[uCurButtonNum].x;
		fT2 = CPauseScreen_avecButtonST2[uCurButtonNum].y;

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
		m_avtxButton[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
		m_avtxButton[uCurVtx].ST.Set(fS2, fT1);

		m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
		m_avtxButton[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
		m_avtxButton[uCurVtx].ST.Set(fS1, fT2);

		m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
		m_avtxButton[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
		m_avtxButton[uCurVtx].ST.Set(fS2, fT2);

		++uCurVtx;

		oTextArea.fUpperLeftX = (fX1 + CPauseScreen_fButtonWidth + 1.0f) * 0.5f;
		oTextArea.fLowerRightX = oTextArea.fUpperLeftX + CPauseScreen_fButtonTextWidth * 0.5f;
		oTextArea.fUpperLeftY = (0.75f - fY1 + 0.03f) * 0.5f;
		oTextArea.fLowerRightY = (0.75f - fY2 - 0.02f + 0.03f) * 0.5f;

		m_ahButtonDesc[uCurButtonNum] = ftext_Create(&oTextArea);

		fX1 += 1.48f;//CPauseScreen_fButtonWidth;
		fX2 += 1.48f;//CPauseScreen_fButtonWidth;
	}

	fS1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_START].x;
	fT1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_START].y;
	fS2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_START].x;
	fT2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_START].y;

	#define _MAG_DELTA		0.01f
	#define _SHIFT_DELTA	-0.02f

	m_avtxButton[uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS = m_avtxButton[6].Pos_MS + CFVec3( -_MAG_DELTA + _SHIFT_DELTA, _MAG_DELTA, 0.0f );
	m_avtxButton[uCurVtx].ST.Set(fS1, fT1);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS = m_avtxButton[7].Pos_MS + CFVec3( _MAG_DELTA + _SHIFT_DELTA, _MAG_DELTA, 0.0f );
	m_avtxButton[uCurVtx].ST.Set(fS2, fT1);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS = m_avtxButton[8].Pos_MS + CFVec3( -_MAG_DELTA + _SHIFT_DELTA, -_MAG_DELTA, 0.0f );
	m_avtxButton[uCurVtx].ST.Set(fS1, fT2);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS = m_avtxButton[9].Pos_MS + CFVec3( _MAG_DELTA + _SHIFT_DELTA, _MAG_DELTA, 0.0f );
	m_avtxButton[uCurVtx].ST.Set(fS2, fT1);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS = m_avtxButton[10].Pos_MS + CFVec3( -_MAG_DELTA + _SHIFT_DELTA, -_MAG_DELTA, 0.0f );
	m_avtxButton[uCurVtx].ST.Set(fS1, fT2);

	m_avtxButton[++uCurVtx].ColorRGBA.OpaqueWhite();
	m_avtxButton[uCurVtx].Pos_MS = m_avtxButton[11].Pos_MS + CFVec3( _MAG_DELTA + _SHIFT_DELTA, -_MAG_DELTA, 0.0f );
	m_avtxButton[uCurVtx].ST.Set(fS2, fT2);

	++uCurVtx;
	//
	////

//	InitLevel();

	_fUnitFlash = 0.0f;
	_nActiveWrapperScreen = WPR_DATATYPES_SCREENS_NONE;

#if defined(MA_PC_INPUT)
	m_MousePointerTex.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfh_cross01" ) );
#endif

	m_eState = PSSTATE_INACTIVE;

	return(TRUE);
}

// =============================================================================================================

void CPauseScreen::UninitSystem()
{
	u32 uMIIdx;
	for(uMIIdx = 0; uMIIdx < 14; ++uMIIdx)
		m_aMIWeaponPrimary[uMIIdx].Uninit();
	for(uMIIdx = 0; uMIIdx < 14; ++uMIIdx)
		m_aMIWeaponSecondary[uMIIdx].Uninit();
	for(uMIIdx = 0; uMIIdx < 6; ++uMIIdx)
		m_aMIItem[uMIIdx].Uninit();
	for(uMIIdx = 0; uMIIdx < 6; ++uMIIdx)
		m_aMIOptions[uMIIdx].Uninit();
#if 0
	for(uMIIdx = 0; uMIIdx < 4; ++uMIIdx)
		m_aMIYesNo[uMIIdx].Uninit();
#endif
	m_MIBlueBox.Uninit();
#if defined(MA_PC_INPUT)
	m_MousePointerTex.SetTexDef( NULL );
#endif
}

// =============================================================================================================

BOOL CPauseScreen::LevelInit()
{
	m_bIsEnabled = TRUE;
	SetWSEnable(TRUE);

//	m_fEUKHighlightTimer = 0.0f;
	m_eState = PSSTATE_INACTIVE;

	// make the options screen the default, and the "restart level" the current selection
	m_aMS[0].ForceCurrentItem( 0 );
	m_MenuMgr.SetCurrent(&m_aMS[0]);

	_fUnitFlash = 0.0f;
	_nActiveWrapperScreen = WPR_DATATYPES_SCREENS_NONE;

	return(TRUE);
}

// =============================================================================================================

void CPauseScreen::LevelUninit()
{
	floop_PauseGame(FALSE);
}

// =============================================================================================================

BOOL CPauseScreen::IsActive()
{
	return(m_eState != PSSTATE_INACTIVE);
}

// =============================================================================================================

void CPauseScreen::SetEnabled(BOOL bEnabled)
{
	m_bIsEnabled = bEnabled;
}

// =============================================================================================================

BOOL CPauseScreen::Start(CInventory *pInventory )
{
	if( pInventory == NULL )
	{
		return(FALSE);
	}

	if(!m_bIsEnabled)
	{
		return(FALSE);
	}

//	if(CHud2::m_eWeaponSelectState != WEAPONSELECTSTATE_IDLE) {
//		return(FALSE);
//	}

	if(m_eState != PSSTATE_INACTIVE)
	{
		return(FALSE);
	}

#if _4_SCREEN_SETUP
	if( (CHud2::GetHudForPlayer(0)->m_eCurHudMode == HUDMODE_MIL) ||
		(CHud2::GetHudForPlayer(0)->m_eCurHudMode == HUDMODE_DROIDFRIEND) ) {
		m_MenuMgr.SetCurrent(&m_aMS[3]);
		m_aMS[3].m_apNext[0] = NULL;
		m_aMS[3].m_apNext[1] = NULL;
	} else {
		m_aMS[3].m_apNext[0] = &m_aMS[2];
		m_aMS[3].m_apNext[1] = &m_aMS[0];
	}
#else
	if( CHud2::GetHudForPlayer(0)->m_eCurHudMode != HUDMODE_GLITCH) {
		// disable the items inventory screen
		m_MenuMgr.SetCurrent(&m_aMS[0]);
		m_aMS[0].m_apNext[0] = NULL;// left
		m_aMS[0].m_apNext[1] = NULL;// right
	} else {
		m_aMS[0].m_apNext[0] = &m_aMS[1];// left
		m_aMS[0].m_apNext[1] = &m_aMS[1];// right
	}
#endif

//	if(m_eState == PSSTATE_INACTIVE)
	{
		m_eState = PSSTATE_NORMAL;

		//// Set up the CMenuItems to either point to data in their inventory, or the null item.
		//
		u32 uX, uY, uCurMIIdx;
		u32 uCurWidth, uCurHeight;

		// For the primary weapon screen.
		uCurWidth = CPauseScreen_auScreenItemWidth[0];
		uCurHeight = CPauseScreen_auScreenItemHeight[0];
		for(uY = 0; uY < uCurHeight; ++uY)
		{
			for(uX = 0; uX < uCurWidth; ++uX)
			{
				uCurMIIdx = (uY * uCurWidth) + uX;
				m_aMIWeaponPrimary[uCurMIIdx].InitWithItemInst(&m_aoNullSlot[0]);
			}
		}

		// For the secondary weapon screen.
		uCurWidth = CPauseScreen_auScreenItemWidth[1];
		uCurHeight = CPauseScreen_auScreenItemHeight[1];
		for(uY = 0; uY < uCurHeight; ++uY)
		{
			for(uX = 0; uX < uCurWidth; ++uX)
			{
				uCurMIIdx = (uY * uCurWidth) + uX;
				m_aMIWeaponSecondary[uCurMIIdx].InitWithItemInst(&m_aoNullSlot[1]);
			}
		}

		// For the item screen.
		uCurWidth = CPauseScreen_auScreenItemWidth[2];
		uCurHeight = CPauseScreen_auScreenItemHeight[2];
		for(uY = 0; uY < uCurHeight; ++uY)
		{
			for(uX = 0; uX < uCurWidth; ++uX)
			{
				uCurMIIdx = (uY * uCurWidth) + uX;
				m_aMIItem[uCurMIIdx].InitWithItemInst(&m_aoNullSlot[2]);
			}
		}

		CItemInst *pII;
		CItem *pI;
		u32 uInventoryIdx, uInventorySize;
			
		uInventorySize = pInventory->m_auNumWeapons[0];
		uCurWidth = CPauseScreen_auScreenItemWidth[0];
		uCurHeight = CPauseScreen_auScreenItemHeight[0];
		for(uInventoryIdx = 0; uInventoryIdx < uInventorySize; ++uInventoryIdx)
		{
			pII = &(pInventory->m_aoWeapons[0][uInventoryIdx]);
			pI = pII->m_pItemData;
			if(pI == NULL)
			{
				continue;
			}
			uX = pI->m_uInventoryPos_x;
			FASSERT(uX < uCurWidth);
			uY = pI->m_uInventoryPos_y;
			FASSERT(uY < uCurHeight);
			uCurMIIdx = (uY * uCurWidth) + uX;
			m_aMIWeaponPrimary[uCurMIIdx].InitWithItemInst(pII);
		}

		uInventorySize = pInventory->m_auNumWeapons[1];
		uCurWidth = CPauseScreen_auScreenItemWidth[1];
		uCurHeight = CPauseScreen_auScreenItemHeight[1];
		for(uInventoryIdx = 0; uInventoryIdx < uInventorySize; ++uInventoryIdx)
		{
			pII = &(pInventory->m_aoWeapons[1][uInventoryIdx]);
			pI = pII->m_pItemData;
			if(pI == NULL)
			{
				continue;
			}
			uX = pI->m_uInventoryPos_x;
			FASSERT(uX < uCurWidth);
			uY = pI->m_uInventoryPos_y;
			FASSERT(uY < uCurHeight);
			uCurMIIdx = (uY * uCurWidth) + uX;
			m_aMIWeaponSecondary[uCurMIIdx].InitWithItemInst(pII);
		}

		//ME well, this isn't pretty, but if you're possessing a mil, you probably don't have currently selected inventory
		if(CHud2::GetHudForPlayer(0)->m_eCurHudMode != HUDMODE_MIL) {
			RefreshSelected(pInventory);
		}

		uInventorySize = pInventory->m_uNumItems;
		uCurWidth = CPauseScreen_auScreenItemWidth[2];
		uCurHeight = CPauseScreen_auScreenItemHeight[2];
		for(uInventoryIdx = 0; uInventoryIdx < uInventorySize; ++uInventoryIdx)
		{
			pII = &(pInventory->m_aoItems[uInventoryIdx]);
			pI = pII->m_pItemData;
			if(pI == NULL)
			{
				continue;
			}
			uX = pI->m_uInventoryPos_x;
			FASSERT(uX < uCurWidth);
			uY = pI->m_uInventoryPos_y;
			FASSERT(uY < uCurHeight);
			uCurMIIdx = uY * uCurWidth + uX;
			m_aMIItem[uCurMIIdx].InitWithItemInst(pII);
		}		
		//
		////

		m_bHUDEnabled = CHud2::GetHudForPlayer(0)->IsDrawEnabled();
		CHud2::GetHudForPlayer(0)->SetDrawEnabled(FALSE);

		m_uButtons |= PSINPUT_START;

		floop_PauseGame( TRUE );
	}

	// remember draw state of script system text monitors, then turn them off
	m_bShowMonitors = CFScriptSystem::GetMonitorsOn();
	CFScriptSystem::SetMonitorsOn( FALSE );

	// JUSTIN: Add in that faudio call here.
	//Pause all currently playing sounds and streams...
	//CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
	if (m_bPauseAudio)
	{
		CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		CFAudioStream::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_1 );
	}

	return TRUE;
}

// =============================================================================================================
// call this function to exit pause screen
BOOL CPauseScreen::ExitPause( void ) {
	s32 i;

	m_eState = PSSTATE_INACTIVE;
	
	if( m_bHUDEnabled ) {
		CHud2::GetHudForPlayer(0)->SetDrawEnabled(TRUE);
	}

	floop_PauseGame( FALSE );

	// restore draw state of script system text monitors before exiting pause
	CFScriptSystem::SetMonitorsOn( m_bShowMonitors );

	//UnPause all the previously playing sounds and streams...
	CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_NONE );
	CFAudioStream::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_NONE );

	// update the active player's profile settings, not there progress, just things like FFeedback, audio levels, etc...
	for( i=0; i < CPlayer::m_nPlayerCount; ++i ) {
		Player_aPlayer[i].UpdateProfileUserSettings();
		wpr_system_IG_SaveGame( Player_aPlayer[i].m_pPlayerProfile );
	}

	return TRUE;
}

// =============================================================================================================

void CPauseScreen::SetWSEnable(BOOL bWSEnabled)
{
	m_bWSEnabled = bWSEnabled;
}

// =============================================================================================================


void CPauseScreen::Work(CInventory *pInventory)
{
	s32 i;

	if( m_bQuitNextFrame ) {
		launcher_EnterMenus( LAUNCHER_FROM_GAME );
		// restore draw state of script system text monitors before exiting level
		CFScriptSystem::SetMonitorsOn( m_bShowMonitors );
		m_bQuitNextFrame = FALSE;
	}

	if( m_eState == PSSTATE_INACTIVE ) {
		return;
	}

	FTextArea_t *pTextArea = ftext_GetAttributes( m_hMissionTitle );
	if( pTextArea ) {
		pTextArea->bVisible = FALSE;
	}

	pTextArea = ftext_GetAttributes( m_hMissionText );
	if( pTextArea ) {
		pTextArea->bVisible = FALSE;
	}

	// Make sure the streaming data is back in the cache before we start to render, again...
	fvis_BlockOnStreamingData();

	// check whether we're waiting for the messagebox to finish
	if( m_bMsgBoxActive ) {
		if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
			switch( CMsgBox::GetContextData() ) {
			case _MSGBOX_DATA_EXIT:
				for( i=0; i < CPlayer::m_nPlayerCount; ++i ) {
					Player_aPlayer[i].UpdateProfileUserSettings();
					wpr_system_IG_SaveGame( Player_aPlayer[i].m_pPlayerProfile );
				}

				m_bQuitNextFrame = TRUE;

				break;

			case _MSGBOX_DATA_RESTART:
			case _MSGBOX_DATA_RESPAWN:
				//Stop any sounds that might have currently been playing
				CFAudioEmitter::StopAll();

				if( (CMsgBox::GetContextData() == _MSGBOX_DATA_RESTART) || !checkpoint_Saved( 1 ) ) {
					// Restart level...
					checkpoint_Restore( 0, FALSE );
					checkpoint_SetUnsaved( 1 );
				} else {
					// Respawn to last checkpoint...

					FASSERT( checkpoint_Saved( 1 ) );
					checkpoint_Restore( 1, FALSE );
				}

				// Leave the menu system.
				ExitPause();

				// invoke the level intro movie
				level_PlayIntroMovie(); 

				break;

			default:
				FASSERT_NOW;
			};
		}

		m_bMsgBoxActive = FALSE;
		CMsgBox::ClearButtonPress();
		GetControls();
		m_MenuMgr.GetControls();
	}

	if( _nActiveWrapperScreen != WPR_DATATYPES_SCREENS_NONE ) {
		// In-game wrappers have control...

		if( wpr_system_IG_Work() ) {
			// Wrapper screen has exited...

			_nActiveWrapperScreen = WPR_DATATYPES_SCREENS_NONE;

			GetControls();
			m_MenuMgr.GetControls();
		}

		return;
	}

	_fUnitFlash += _FLASHING_SHOULDER_ICON_SPEED * FLoop_fRealPreviousLoopSecs;
	if( _fUnitFlash >= FMATH_2PI ) {
		_fUnitFlash = 0.0f;
	}

	if( !m_bIgnoreControlsNextFrame ) {
		GetControls();
	} else {
		GetControls();
		GetControls();
		m_bIgnoreControlsNextFrame = FALSE;
	}

	if( CMsgBox::IsActive() ) {
		m_bIgnoreControlsNextFrame = TRUE;
	}

	

	m_pInventory = pInventory;

	if( (_nActiveWrapperScreen == WPR_DATATYPES_SCREENS_NONE) && (m_MenuMgr.GetState() != MMSTATE_SCROLLING) ) {
		if( (m_uButtonsLatched & PSINPUT_START) == PSINPUT_START ) {
			ExitPause();
			return;
		}
	}

	m_MenuMgr.Work();

	CMenuScreen *pCurMS = m_MenuMgr.m_pCurMS;
	FASSERT(pCurMS != NULL);
	CMenuItem *pCurMI = pCurMS->m_pCurMI;
	FASSERT(pCurMI != NULL);

#if defined(MA_PC_INPUT)
	f32 fPointerX, fPointerY;
	if( pcinput_MenuPointer( &fPointerX, &fPointerY ) ) {
		pcinput_DrawsMenuPointer();

		f32 fClickX = 0.0f, fClickY = 0.0f;
		BOOL bClick = pcinput_TakeMenuClick( FALSE, &fClickX, &fClickY );
		BOOL bRightClick = pcinput_TakeMenuClick( TRUE, NULL, NULL );
		s32 nWheel = pcinput_TakeMenuWheel();

		if( bRightClick ) {
			if( _nActiveWrapperScreen == WPR_DATATYPES_SCREENS_NONE ) {
				ExitPause();
				return;
			}
		}

		if( _nActiveWrapperScreen == WPR_DATATYPES_SCREENS_NONE ) {
			if( bClick ) {
				// Tab L: X in [0.07f, 0.19f], Y in [0.04f, 0.18f]
				if( fClickX >= 0.07f && fClickX <= 0.19f && fClickY >= 0.04f && fClickY <= 0.18f ) {
					m_MenuMgr.ScrollLeft();
					bClick = FALSE;
				}
				// Tab R: X in [0.81f, 0.93f], Y in [0.04f, 0.18f]
				else if( fClickX >= 0.81f && fClickX <= 0.93f && fClickY >= 0.04f && fClickY <= 0.18f ) {
					m_MenuMgr.ScrollRight();
					bClick = FALSE;
				}
				// Bottom Prompt 1: Resume Game / Cancel
				else if( fClickY >= 0.84f && fClickY <= 0.98f && fClickX >= 0.34f && fClickX <= 0.72f ) {
					ExitPause();
					return;
				}
				// Bottom Prompt 0: Select / Accept
				else if( fClickY >= 0.84f && fClickY <= 0.98f && fClickX >= 0.08f && fClickX <= 0.33f ) {
					if( m_MenuMgr.GetState() == MMSTATE_STATIC ) {
						pCurMS->ButtonPressed( MMINPUT_BOTTOMBUTTON );
						fsndfx_Play2D( CMenuMgr::m_hClickSnd );
						bClick = FALSE;
					}
				}
			}

			// Item hit testing on current screen
			if( m_MenuMgr.GetState() == MMSTATE_STATIC ) {
				for( u32 idx = 0; idx < pCurMS->m_uNumMI; ++idx ) {
					CMenuItem *pMI = pCurMS->m_apMI[idx];
					if( !pMI ) continue;

					// Bounding box in ortho coordinates (-1..1, -0.75..0.75)
					f32 fL = (pMI->m_vecBorderUL.x + 1.0f) * 0.5f;
					f32 fR = fL + pMI->m_vecBorderRect.x * 0.5f;
					f32 fT = (0.75f - pMI->m_vecBorderUL.y) / 1.5f;
					f32 fB = fT + pMI->m_vecBorderRect.y / 1.5f;

					fL -= 0.008f; fR += 0.008f;
					fT -= 0.008f; fB += 0.008f;

					if( fPointerX >= fL && fPointerX <= fR && fPointerY >= fT && fPointerY <= fB ) {
						if( pcinput_MenuPointerMoved() && pCurMS->m_pCurMI != pMI ) {
							pCurMS->ForceCurrentItem( idx );
							m_MenuMgr.UpdateCursorToCurrent();
							fsndfx_Play2D( CMenuMgr::m_hSelectSnd );
						}

						if( bClick && fClickX >= fL && fClickX <= fR && fClickY >= fT && fClickY <= fB ) {
							pCurMS->ForceCurrentItem( idx );
							m_MenuMgr.UpdateCursorToCurrent();
							pCurMS->ButtonPressed( MMINPUT_BOTTOMBUTTON );
							fsndfx_Play2D( CMenuMgr::m_hClickSnd );
							bClick = FALSE;
							break;
						}
					}
				}

				if( nWheel != 0 ) {
					pCurMS->Move( nWheel > 0 ? MIDIR_UP : MIDIR_DOWN );
					m_MenuMgr.UpdateCursorToCurrent();
				}
			}
		}
	}
#endif

	if( m_eState == PSSTATE_NORMAL ) {
		if( m_MenuMgr.GetState() != MMSTATE_SCROLLING ) {
			m_fItemTheta += FLoop_fRealPreviousLoopSecs * CPauseScreen_fItemOmegaYaw;
		}
	}

	m_pInventory = NULL;

	if( m_MenuMgr.GetState() != MMSTATE_SCROLLING ) {
		if( _nActiveWrapperScreen == WPR_DATATYPES_SCREENS_NONE ) {
			if( pCurMS == &m_aMS[0] ) {
				// Main menu...

				CPauseScreen_apwszButtonText[0] = Game_apwszPhrases[ GAMEPHRASE_ACCEPT ];
				CPauseScreen_apwszButtonText[1] = Game_apwszPhrases[ GAMEPHRASE_RESUME_GAME ];
			} else {
				// Info screen...

				CPauseScreen_apwszButtonText[0] = NULL;
				CPauseScreen_apwszButtonText[1] = Game_apwszPhrases[ GAMEPHRASE_RESUME_GAME ];
			}
		} else {
			CPauseScreen_apwszButtonText[0] = Game_apwszPhrases[ GAMEPHRASE_ACCEPT ];
			CPauseScreen_apwszButtonText[1] = Game_apwszPhrases[ GAMEPHRASE_CANCEL ];
		}
	} else {
		CPauseScreen_apwszButtonText[0] = NULL;
		CPauseScreen_apwszButtonText[1] = NULL;
	}
}

// =============================================================================================================

void CPauseScreen::Draw(CInventory *pInventory)
{
	if( m_eState == PSSTATE_INACTIVE ) {
		return;
	}

	CMenuScreen *pCurMS = m_MenuMgr.m_pCurMS;
	FASSERT(pCurMS != NULL);

	CMenuItem *pCurMI = pCurMS->m_pCurMI;
	FASSERT(pCurMI != NULL);

	CFXfm oCamera;
	FViewport_t *pviewPrevious = fviewport_GetActive();

	fviewport_SetActive(m_pviewOrtho3d);

//	fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.1f, 0.1f, 0.1f, 1.0f, 0);
	fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.016f, 0.169f, 1.0f, 0);
//	CFColorRGBA rgbaTemp(0.0f, 0.016f, 0.169f, 0.4f);

	frenderer_Push(FRENDERER_DRAW, NULL);

		oCamera.Identity();
		oCamera.InitStackWithView();

		fdraw_Depth_EnableWriting(FALSE);
		fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
		fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);

		//// Set up transformation to convert from screen coordinates to pixel coordinates (see comments below).
		//
		CFXfm xfmWindow, xfmTemp;
		// This translation is here, well, really, we don't need it if we move the near plane up.  But basically, all of the
		//   z-coordinates that you provide ANYWHERE in this class must be 0.0f so that they don't get scaled by the uniform
		//   scaling matrices that we have to use.
		xfmWindow.BuildTranslation(0.0f, 0.0f, 1.0f);
		// This is used to normalize the coordinates that I will be using (x E [-1.0f, 1.0f], y E [-0.75f, 0.75f])
		// This assumes a 4:3, x:y, window ratio, if otherwise, each axis must be scaled differently.
		xfmTemp.BuildScale(pviewPrevious->HalfRes.x);
		xfmWindow.ReceiveProductOf(xfmWindow, xfmTemp);
		xfmWindow.PushModel();
		//
		////

			////
			//
			fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);

			CPauseScreen::DrawFrame();
			if( !CMsgBox::IsActive() && (_nActiveWrapperScreen == WPR_DATATYPES_SCREENS_NONE) ) {
				m_MenuMgr.Draw();
			}

			fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
			if(m_MenuMgr.GetState() == MMSTATE_STATIC)
			{
				fdraw_SetTexture(&m_texHL);

				if(m_MenuMgr.GetState() == MMSTATE_STATIC)
				{
					DrawScreenTitle();
				}

				if(m_eState == PSSTATE_NORMAL)
				{
					if((pCurMS == &(m_aMS[0])) || (pCurMS == &(m_aMS[1])) || (pCurMS == &(m_aMS[2])))
					{
						DrawInfoBoxText(pCurMI, pInventory);
					}

					DrawMissionText();
				}
			}
			//
			////

		CFXfm::PopModel();

	frenderer_Pop();

	if( _nActiveWrapperScreen != WPR_DATATYPES_SCREENS_NONE ) {
		wpr_system_IG_Draw();
		return;
	}

	fviewport_SetActive(m_pviewPerspective);
	fviewport_Clear(FVIEWPORT_CLEARFLAG_DEPTH | FVIEWPORT_CLEARFLAG_STENCIL, 0.0f, 0.0f, 0.0f, 1.0f, 0);

	frenderer_Push(FRENDERER_MESH, NULL);

//		oCamera.Identity();
		oCamera.InitStackWithView();

		if(m_eState == PSSTATE_NORMAL)
		{
			if(m_MenuMgr.GetState() != MMSTATE_SCROLLING)
			{
				if((pCurMS == &(m_aMS[0])) || (pCurMS == &(m_aMS[1])) || (pCurMS == &(m_aMS[2])))
				{
					DrawInfoBoxMesh(pCurMI);
				}
			}
		}

	frenderer_Pop();

#if defined(MA_PC_INPUT)
	f32 fMouseX, fMouseY;
	if( pcinput_MenuPointer( &fMouseX, &fMouseY ) && m_MousePointerTex.GetTexDef() ) {
		fviewport_SetActive( m_pviewOrtho3d );
		frenderer_Push( FRENDERER_DRAW, NULL );

		oCamera.Identity();
		oCamera.InitStackWithView();

		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		CFXfm xfmCursor, xfmCursorScale;
		xfmCursor.BuildTranslation( 0.0f, 0.0f, 1.0f );
		xfmCursorScale.BuildScale( pviewPrevious->HalfRes.x );
		xfmCursor.ReceiveProductOf( xfmCursor, xfmCursorScale );
		xfmCursor.PushModel();

		fdraw_SetTexture( &m_MousePointerTex );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );

		f32 fApexX = fMouseX * 2.0f - 1.0f;
		f32 fApexY = 0.75f - fMouseY * 1.5f;
		f32 fHalfY = 0.5f * (0.10f * 0.75f);
		f32 fHalfX = fHalfY;
		f32 fCenterY = fApexY - (0.5f - 0.07f) * (0.10f * 0.75f);
		f32 fLowerX = fApexX - fHalfX, fUpperX = fApexX + fHalfX;
		f32 fLowerY = fCenterY - fHalfY, fUpperY = fCenterY + fHalfY;

		FDrawVtx_t aVtx[4];
		aVtx[0].ST.Set( 0.0f, 1.0f );
		aVtx[1].ST.Set( 0.0f, 0.0f );
		aVtx[2].ST.Set( 1.0f, 1.0f );
		aVtx[3].ST.Set( 1.0f, 0.0f );

		f32 fPixelX = 1.5f / pviewPrevious->HalfRes.x;
		f32 fPixelY = 1.5f / pviewPrevious->HalfRes.y;
		CFColorRGBA shadowCol( 0.0f, 0.0f, 0.05f, 0.85f );
		for( int pass = 0; pass < 5; ++pass ) {
			f32 ox = 0.0f, oy = 0.0f;
			CFColorRGBA col;
			if( pass == 0 ) { ox = -fPixelX; col = shadowCol; }
			else if( pass == 1 ) { ox = fPixelX; col = shadowCol; }
			else if( pass == 2 ) { oy = -fPixelY; col = shadowCol; }
			else if( pass == 3 ) { oy = fPixelY; col = shadowCol; }
			else { col.Set( 1.0f, 1.0f, 1.0f, 1.0f ); }

			aVtx[0].Pos_MS.Set( fLowerX + ox, fLowerY + oy, 0.0f );
			aVtx[1].Pos_MS.Set( fLowerX + ox, fUpperY + oy, 0.0f );
			aVtx[2].Pos_MS.Set( fUpperX + ox, fLowerY + oy, 0.0f );
			aVtx[3].Pos_MS.Set( fUpperX + ox, fUpperY + oy, 0.0f );
			for( int i = 0; i < 4; ++i ) aVtx[i].ColorRGBA = col;
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );
		}

		CFXfm::PopModel();
		frenderer_Pop();
		pcinput_DrawsMenuPointer();
	}
#endif

	fviewport_SetActive( pviewPrevious );
}

// =============================================================================================================

void CPauseScreen::GetControls()
{
	u32 uLastButtons = m_uButtons;

	m_uButtons = 0;

	if( Gamepad_aapSample[ Player_aPlayer[CPlayer::m_nCurrent].m_nControllerIndex ][GAMEPAD_MAIN_PAUSE]->uLatches & FPAD_LATCH_ON ) {
		m_uButtons |= PSINPUT_START;
	}
#if defined(MA_PC_INPUT)
	if( (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0 ) {
		m_uButtons |= PSINPUT_START;
	}
#endif

	m_uButtonsLatched = m_uButtons & (~uLastButtons);
}

// =============================================================================================================

void CPauseScreen::SetupFrame()
{
	u32 uTemp;
	for(uTemp = 0; uTemp < 2; ++uTemp)
	{
		m_atexFrame[uTemp].SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, CPauseScreen_apszFrameTex[uTemp])) );
		if(m_atexFrame[uTemp].GetTexDef() == NULL)
		{
			DEVPRINTF("CPauseScreen::InitSystem : Could not load frame texture '%s'.\n", CPauseScreen_apszFrameTex[uTemp]);
		}

		m_atexFrame[uTemp].ClearFlag( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T | CFTexInst::FLAG_WRAP_U );
	}

	f32 fX1, fX2, fY1, fY2;
	f32 fS1, fS2, fT1, fT2;

	u32 uSide;
	for(uSide = 0; uSide < 2; ++uSide)
	{
		fX1 = CPauseScreen_avecFrameUL[uSide].x;
		fX2 = CPauseScreen_avecFrameBR[uSide].x;
		fY1 = CPauseScreen_avecFrameUL[uSide].y;
		fY2 = CPauseScreen_avecFrameBR[uSide].y;

		fS1 = CPauseScreen_avecFrameSTUL[uSide].x;
		fS2 = CPauseScreen_avecFrameSTLR[uSide].x;
		fT1 = CPauseScreen_avecFrameSTUL[uSide].y;
		fT2 = CPauseScreen_avecFrameSTLR[uSide].y;

		m_avtxFrame[0 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[0 + uSide * 12].Pos_MS.Set(fX1, fY1, 0.0f);
		m_avtxFrame[0 + uSide * 12].ST.Set(fS1, fT1);

		m_avtxFrame[1 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[1 + uSide * 12].Pos_MS.Set(fX2, fY1, 0.0f);
		m_avtxFrame[1 + uSide * 12].ST.Set(fS2, fT1);

		m_avtxFrame[2 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[2 + uSide * 12].Pos_MS.Set(fX1, fY2, 0.0f);
		m_avtxFrame[2 + uSide * 12].ST.Set(fS1, fT2);

		m_avtxFrame[3 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[3 + uSide * 12].Pos_MS.Set(fX2, fY1, 0.0f);
		m_avtxFrame[3 + uSide * 12].ST.Set(fS2, fT1);

		m_avtxFrame[4 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[4 + uSide * 12].Pos_MS.Set(fX1, fY2, 0.0f);
		m_avtxFrame[4 + uSide * 12].ST.Set(fS1, fT2);

		m_avtxFrame[5 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[5 + uSide * 12].Pos_MS.Set(fX2, fY2, 0.0f);
		m_avtxFrame[5 + uSide * 12].ST.Set(fS2, fT2);

		u32 uTempOfs;
		u32 uSrcIdx, uDestIdx;
		uSrcIdx = uSide * 12;
		uDestIdx = uSide * 12 + 6;
		for(uTempOfs = 0; uTempOfs < 6; ++uTempOfs)
		{
			m_avtxFrame[uDestIdx] = m_avtxFrame[uSrcIdx];
			m_avtxFrame[uDestIdx].Pos_MS.x *= -1.0f;
			++uSrcIdx;
			++uDestIdx;
		}
	}
}

// =============================================================================================================

void CPauseScreen::DrawFrame()
{
#if 0
//xxxxxxxxxxxxxxx
{
	f32 fX1, fX2, fY1, fY2;
	f32 fS1, fS2, fT1, fT2;

	u32 uSide;

	CFVec2 avecFrameSTUL[2] = { CFVec2(-1.0f, 0.0f), CFVec2(-1.0f, 0.75f) };
	CFVec2 avecFrameSTLR[2] = { CFVec2(2.0f, 0.70f), CFVec2(1.5f, 1.3f) };

	for(uSide = 0; uSide < 2; ++uSide)
	{
		fX1 = CPauseScreen_avecFrameUL[uSide].x;
		fX2 = CPauseScreen_avecFrameBR[uSide].x;
		fY1 = CPauseScreen_avecFrameUL[uSide].y;
		fY2 = CPauseScreen_avecFrameBR[uSide].y;

		fS1 = avecFrameSTUL[uSide].x;
		fS2 = avecFrameSTLR[uSide].x;
		fT1 = avecFrameSTUL[uSide].y;
		fT2 = avecFrameSTLR[uSide].y;

		m_avtxFrame[0 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[0 + uSide * 12].Pos_MS.Set(fX1, fY1, 0.0f);
		m_avtxFrame[0 + uSide * 12].ST.Set(fS1, fT1);

		m_avtxFrame[1 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[1 + uSide * 12].Pos_MS.Set(fX2, fY1, 0.0f);
		m_avtxFrame[1 + uSide * 12].ST.Set(fS2, fT1);

		m_avtxFrame[2 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[2 + uSide * 12].Pos_MS.Set(fX1, fY2, 0.0f);
		m_avtxFrame[2 + uSide * 12].ST.Set(fS1, fT2);

		m_avtxFrame[3 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[3 + uSide * 12].Pos_MS.Set(fX2, fY1, 0.0f);
		m_avtxFrame[3 + uSide * 12].ST.Set(fS2, fT1);

		m_avtxFrame[4 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[4 + uSide * 12].Pos_MS.Set(fX1, fY2, 0.0f);
		m_avtxFrame[4 + uSide * 12].ST.Set(fS1, fT2);

		m_avtxFrame[5 + uSide * 12].ColorRGBA.OpaqueWhite();
		m_avtxFrame[5 + uSide * 12].Pos_MS.Set(fX2, fY2, 0.0f);
		m_avtxFrame[5 + uSide * 12].ST.Set(fS2, fT2);

		u32 uTempOfs;
		u32 uSrcIdx, uDestIdx;
		uSrcIdx = uSide * 12;
		uDestIdx = uSide * 12 + 6;
		for(uTempOfs = 0; uTempOfs < 6; ++uTempOfs)
		{
			m_avtxFrame[uDestIdx] = m_avtxFrame[uSrcIdx];
			m_avtxFrame[uDestIdx].Pos_MS.x *= -1.0f;
			++uSrcIdx;
			++uDestIdx;
		}
	}
}
#endif
	// TODO : This can now be rendered in one PrimList call.
	fdraw_SetTexture(&m_atexFrame[0]);
	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, &(m_avtxFrame[0]), 12);
	fdraw_SetTexture(&m_atexFrame[1]);
	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, &(m_avtxFrame[12]), 12);

	fdraw_SetTexture(&m_texControls);

	u32 uCurButton, nButtonIndex;

	if( !CMsgBox::IsActive() ) {
		for( uCurButton = 0; uCurButton < CPauseScreen_uNumButtonsDisplayed; ++uCurButton ) {
			if( CPauseScreen_apwszButtonText[uCurButton] ) {
				ftext_PrintString( m_ahButtonDesc[uCurButton], CPauseScreen_apwszButtonText[uCurButton] );

				// Assume "A" button...
				nButtonIndex = 0;

				if( uCurButton ) {
					// "B" or "Start" button...

					if( _nActiveWrapperScreen == WPR_DATATYPES_SCREENS_NONE ) {
						// Main menu or info screen. Use "Start" button...

						nButtonIndex = 4;
					} else {
						// In-game wrappers. Use "B" button...

						nButtonIndex = 1;
					}
				}

				fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, &(m_avtxButton[6 * nButtonIndex]), 6 );
			}
		}

		if( _nActiveWrapperScreen == WPR_DATATYPES_SCREENS_NONE ) {
			if( CHud2::GetHudForPlayer(0)->m_eCurHudMode != HUDMODE_MIL ) {
				u32 i;
				f32 fAlpha;

				fAlpha = 0.18f + 0.8f * fmath_PositiveSin( _fUnitFlash );

				for( i=0; i<12; ++i ) {
					m_avtxButton[12+i].ColorRGBA.fAlpha = fAlpha;
				}

				fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, &(m_avtxButton[6 * 2]), 6 * 2);
			}
		}
	}
}

// =============================================================================================================

void CPauseScreen::WeaponCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData)
{
	switch(eReason)
	{
		case MIREASON_BUTTONPRESSED:
		{
			// SER: This is Justin's "clean" way of determining which menu screen we're on. OMG what a kludgy hack!
			u32 uWhichWeapon;
			if(m_MenuMgr.m_pCurMS == &(m_aMS[0]))
			{
				uWhichWeapon = 0;
			}
			else
			{
				FASSERT(m_MenuMgr.m_pCurMS == &(m_aMS[1]));
				uWhichWeapon = 1;
			}

			switch(uData)
			{
				case MMINPUT_BOTTOMBUTTON:
				{
					if(!m_bWSEnabled)
					{
						return;
					}

					if(CHud2::GetHudForPlayer(0)->m_eCurHudMode != HUDMODE_GLITCH)
					{
						return;
					}
					CItemInst *pII = (CItemInst *)(pMI->m_pvUserData);
					FASSERT(pII != NULL);

					// Don't allow them to select the empty slot.
					if(pII == &m_aoNullSlot[uWhichWeapon])
					{
						return;
					}

					m_pInventory->SetCurWeapon(uWhichWeapon, pII, FALSE, TRUE);

					// Update the current cursor positions (this must be done *after* the above step because the above step
					//   might make changes to which item is currently selected).
					RefreshSelected(m_pInventory);

					break;
				}
				case MMINPUT_RIGHTBUTTON:
				{
					// Leave the menu system.
					ExitPause();
					break;
				}
			}
			break;
		}
		case MIREASON_SELECTED:
		{
			break;
		}
		case MIREASON_UNSELECTED:
		{
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================

#if 0
void CPauseScreen::YesNoButtonCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData)
{
	switch(eReason)
	{
		case MIREASON_BUTTONPRESSED:
		{
			// SER: This is Justin's "clean" way of determining which menu screen we're on. OMG what a kludgy hack!
			u32 uWhichWeapon;
			if(m_MenuMgr.m_pCurMS == &(m_aMS[0]))
			{
				uWhichWeapon = 0;
			}
			else
			{
				FASSERT(m_MenuMgr.m_pCurMS == &(m_aMS[1]));
				uWhichWeapon = 1;
			}

			switch(uData)
			{
				case MMINPUT_BOTTOMBUTTON:
				{
					_SetTextItemDraw(uWhichWeapon, TRUE);

					// They've said okay to something.
					if(&(m_aMIYesNo[0]) != pMI)
					{
						// It was on the 'No' button.
						FASSERT(pMI == &(m_aMIYesNo[1]));
						// TODO : Turn on cursor for m_MenuMgr.
						m_MenuMgr.SetDormant(FALSE);
						m_aMIYesNo[2].SetText(0, Game_apwszPhrases[ GAMEPHRASE_UNINITIALIZED ] );
						m_aMIYesNo[0].m_pvUserData = NULL;
						m_eState = PSSTATE_NORMAL;
						return;
					}
					// It was to confirm an EUK upgrade.

					// Get the menu item that was originally selected.
					CMenuItem *pMISel = (CMenuItem *)(pMI->m_pvUserData);
					FASSERT(pMISel != NULL);

					// 
					CItemInst *pII = (CItemInst *)(pMISel->m_pvUserData);
					FASSERT(pII != NULL);

					// SER: Typical Justin crap! How much time do I have to spend finishing his work????????
					// JUSTIN: If we ever get upgradable secondaries the 0 below will need to be replaced.
					u32 nWeaponSide = 0;
					if( (pII->m_pItemData->m_uFlags & (ITEMFLAG_PRIMARY | ITEMFLAG_SECONDARY)) == ITEMFLAG_SECONDARY ) {
						// Upgrading secondary...
						nWeaponSide = 1;
					}

					m_pInventory->UpgradeWeapon(nWeaponSide, pII);
					pMISel->InitWithItemInst(pII);
/*
					CItem *pNewI = pII->m_pItemData->m_pUpgraded;
					FASSERT(pNewI != NULL);

					pII->m_pItemData = pNewI;
					pMISel->InitWithItemInst(pII);

					u32 uNumEUKS = m_pInventory->m_aoItems[INVPOS_EUK].m_nCurAmmo;
					FASSERT(uNumEUKS > 0);
					//// Shift the EUK flags array downward one and remove it.
					//
					u32 uCurItemIdx;
					for(uCurItemIdx = 1; uCurItemIdx < uNumEUKS; ++uCurItemIdx)
						m_pInventory->m_auEUKFlags[uCurItemIdx - 1] = m_pInventory->m_auEUKFlags[uCurItemIdx];

					--m_pInventory->m_aoItems[INVPOS_EUK].m_nCurAmmo;
					//
					////
*/
					// TODO : Turn on cursor for m_MenuMgr.
					m_MenuMgr.SetDormant(FALSE);
					m_aMIYesNo[2].SetText(0, Game_apwszPhrases[ GAMEPHRASE_UNINITIALIZED ] );
					m_aMIYesNo[0].m_pvUserData = NULL;
					m_eState = PSSTATE_NORMAL;

					break;
				}
				case MMINPUT_RIGHTBUTTON:
				{
					_SetTextItemDraw(uWhichWeapon, TRUE);
					m_MenuMgr.SetDormant(FALSE);
					m_aMIYesNo[2].SetText(0, Game_apwszPhrases[ GAMEPHRASE_UNINITIALIZED ] );
					m_aMIYesNo[0].m_pvUserData = NULL;
					m_eState = PSSTATE_NORMAL;
					break;
				}
			}
			break;
		}
	}
}
#endif

// =============================================================================================================

void CPauseScreen::CancelOnlyCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData)
{
	switch(eReason)
	{
		case MIREASON_BUTTONPRESSED:
		{
			switch(uData)
			{
				case MMINPUT_RIGHTBUTTON:
				{
					// Leave the menu system.
					ExitPause();
					break;
				}
			}
			break;
		}
		case MIREASON_SELECTED:
		{
			break;
		}
		case MIREASON_UNSELECTED:
		{
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================

void CPauseScreen::OptionScreenCallback(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData)
{
	switch(eReason)
	{
		case MIREASON_BUTTONPRESSED:
		{
			switch(uData)
			{
				case MMINPUT_RIGHTBUTTON:
				{
					// Leave the menu system.
					ExitPause();
					break;
				}
				case MMINPUT_BOTTOMBUTTON:
				{
					if( pMI == &m_aMIOptions[4] ) {
						// restart...
						CMsgBox::Display( _LOSEDATA_MSGBOX, Game_apwszPhrases[GAMEPHRASE_WARNING], Game_apwszPhrases[GAMEPHRASE_GAME_WONT_BE_SAVED], Game_apwszPhrases[GAMEPHRASE_ACCEPT], Game_apwszPhrases[GAMEPHRASE_CANCEL], NULL, _MSGBOX_DATA_RESTART );
						m_bMsgBoxActive = TRUE;
						fsndfx_Play2D( CMenuMgr::m_hSelectSnd );
					
					} else if( pMI == &m_aMIOptions[3] ) {
						// respawn to last checkpoint...
						CMsgBox::Display( _LOSEDATA_MSGBOX, Game_apwszPhrases[GAMEPHRASE_WARNING], Game_apwszPhrases[GAMEPHRASE_GAME_WONT_BE_SAVED], Game_apwszPhrases[GAMEPHRASE_ACCEPT], Game_apwszPhrases[GAMEPHRASE_CANCEL], NULL, _MSGBOX_DATA_RESPAWN );
						m_bMsgBoxActive = TRUE;
						fsndfx_Play2D( CMenuMgr::m_hSelectSnd );

					} else if( pMI == &m_aMIOptions[5] ) {
						// quit...
						CMsgBox::Display( _LOSEDATA_MSGBOX, Game_apwszPhrases[GAMEPHRASE_WARNING], Game_apwszPhrases[GAMEPHRASE_GAME_WONT_BE_SAVED], Game_apwszPhrases[GAMEPHRASE_ACCEPT], Game_apwszPhrases[GAMEPHRASE_CANCEL], NULL, _MSGBOX_DATA_EXIT );
						m_bMsgBoxActive = TRUE;
						fsndfx_Play2D( CMenuMgr::m_hSelectSnd );
					
					} else if( pMI == &m_aMIOptions[1] ) {
						// Sound options...
						if( wpr_system_AreWrappersReadyToUse() ) {
							_nActiveWrapperScreen = WPR_DATATYPES_SCREENS_SOUND_OPTIONS;
							wpr_system_IG_SelectScreen( _nActiveWrapperScreen );
							fsndfx_Play2D( CMenuMgr::m_hSelectSnd );
						}

					} else if( pMI == &m_aMIOptions[2] ) {
						// Controller Map...
						if( wpr_system_AreWrappersReadyToUse() ) {
							_nActiveWrapperScreen = WPR_DATATYPES_SCREENS_CONTROLLER_SETTINGS;
							wpr_system_IG_SelectScreen( _nActiveWrapperScreen );
							fsndfx_Play2D( CMenuMgr::m_hSelectSnd );
						}

					} else if( pMI == &m_aMIOptions[0] ) {
						// Advanced Controller...
						if( wpr_system_AreWrappersReadyToUse() ) {
							_nActiveWrapperScreen = WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS;
							wpr_system_IG_SelectScreen( _nActiveWrapperScreen );
							fsndfx_Play2D( CMenuMgr::m_hSelectSnd );
						}
					}
					break;
				}
			}
			break;
		}
		case MIREASON_SELECTED:
		{
			break;
		}
		case MIREASON_UNSELECTED:
		{
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================

void CPauseScreen::DrawInfoBoxMesh(CMenuItem *pMI)
{
	CItemInst *pII = (CItemInst *)(pMI->m_pvUserData);

	// We won't draw anything if not appropriate.
	if(pII == NULL)
		return;
	if(IsANullItem(pII))
		return;

	CItem *pI = (CItem *)(pII->m_pItemData);
	FASSERT(pI != NULL);

	pI->DrawMesh(m_fItemTheta);
}

// =============================================================================================================

void CPauseScreen::DrawInfoBoxText(CMenuItem *pMI, CInventory *pInventory) {
	CItemInst *pII = (CItemInst *)(pMI->m_pvUserData);

	// We won't draw anything if not appropriate.
	if( pII == NULL ) {
		return;
	}

	if( IsANullItem(pII) ) {
		return;
	}

	CItem *pI = (CItem *)(pII->m_pItemData);
	FASSERT(pI != NULL);

	ftext_PrintString( m_hInfoName, pI->m_pwszDisplayName );
	ftext_Printf( m_hInfoDesc, L"~o1%ls", pI->m_pwszLongDesc ); 
}


void CPauseScreen::DrawMissionText( void ) {
	if( m_MenuMgr.m_pCurMS != &m_aMS[1] ) {
		return;
	}

	cwchar *pwszMissionText = level_GetCurrentMissionText();
	if( pwszMissionText == NULL ) {
		return;
	}

	ftext_PrintString( m_hMissionTitle, Game_apwszPhrases[ GAMEPHRASE_MISSION_OBJECTIVE ] );
	ftext_Printf( m_hMissionText, L"~C70701099~o1%ls", pwszMissionText ); 

	FTextArea_t *pTextArea = ftext_GetAttributes( m_hMissionTitle );
	if( pTextArea && !CMsgBox::IsActive() ) {
		pTextArea->bVisible = TRUE;
	} else {
		pTextArea->bVisible = FALSE;	
	}

	pTextArea = ftext_GetAttributes( m_hMissionText );
	if( pTextArea && !CMsgBox::IsActive() ) {
		pTextArea->bVisible = TRUE;
	} else {
		pTextArea->bVisible = FALSE;	
	}
}


// =============================================================================================================

BOOL CPauseScreen::IsUpgradeableNow(CMenuItem *pMI, CInventory *pInventory)
{
	return FALSE;

#if 0
	if(pInventory->m_aoItems[INVPOS_EUK].m_nClipAmmo == 0)
	{
		// No EUKs are available.
		return(FALSE);
	}

	FASSERT(pMI != NULL);
	void *pvUserData = pMI->m_pvUserData;
	if(pvUserData != NULL)
	{
		// The MenuItem refers a game item...
		// TODO : Later I should have some sort of typeID field for each CMenuItem.
		CItemInst *pItemInst = (CItemInst *)(pvUserData);
		CItem *pItem = pItemInst->m_pItemData;
		FASSERT(pItem != NULL);
		if(pItem->m_pUpgraded != NULL)
		{
			// There is an upgrade item attached to this item...
			FASSERT(pItem->m_uItemType < 32);
			if(((1 << pItem->m_uItemType) & pInventory->m_auEUKFlags[0]) != 0)
			{
				// The item can be upgraded with the current EUK.
				return(TRUE);
			}
		}
	}
	return(FALSE);
#endif
}

// =============================================================================================================

BOOL CPauseScreen::IsANullItem(CItemInst *pII)
{
	u32 uCurWeap;
	for(uCurWeap = 0; uCurWeap < 3; ++uCurWeap)
	{
		if(pII == &m_aoNullSlot[uCurWeap])
		{
			return(TRUE);
		}
	}
	return(FALSE);
}

// =============================================================================================================

// Positions the 'currently selected' highlight.
void CPauseScreen::RefreshSelected(CInventory *pInventory)
{
	u32 uCurWeap, uIdxSelected, uXSelected, uYSelected, uCurWidth, uCurHeight;
	CItemInst *pIISelected;
	CItem *pISelected;

	for(uCurWeap = 0; uCurWeap < 2; ++uCurWeap)
	{
		uIdxSelected = pInventory->m_auCurWeapon[uCurWeap];
		pIISelected = &(pInventory->m_aoWeapons[uCurWeap][uIdxSelected]);
		pISelected = pIISelected->m_pItemData;
		if (!pISelected)
			continue;
		uXSelected = pISelected->m_uInventoryPos_x;
		uYSelected = pISelected->m_uInventoryPos_y;

		uCurWidth = CPauseScreen_auScreenItemWidth[uCurWeap];
		uCurHeight = CPauseScreen_auScreenItemHeight[uCurWeap];

		if(uCurWeap == 0)
		{
			m_aMIWeaponPrimary[CPauseScreen_auSelectedIdx[uCurWeap]].m_vecBorderUL = m_aMIWeaponPrimary[uYSelected * uCurWidth + uXSelected].m_vecBorderUL;
			m_aMIWeaponPrimary[CPauseScreen_auSelectedIdx[uCurWeap]].m_vecBorderRect = m_aMIWeaponPrimary[uYSelected * uCurWidth + uXSelected].m_vecBorderRect;
			m_aMIWeaponPrimary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageOfs[0] = m_aMIWeaponPrimary[uYSelected * uCurWidth + uXSelected].m_avecImageOfs[0];
			m_aMIWeaponPrimary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageRect[0] = m_aMIWeaponPrimary[uYSelected * uCurWidth + uXSelected].m_avecImageRect[0];
			m_aMIWeaponPrimary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageOfs[1] = m_aMIWeaponPrimary[uYSelected * uCurWidth + uXSelected].m_avecImageOfs[1];
			m_aMIWeaponPrimary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageRect[1] = m_aMIWeaponPrimary[uYSelected * uCurWidth + uXSelected].m_avecImageRect[1];
		}
		else
		{
			m_aMIWeaponSecondary[CPauseScreen_auSelectedIdx[uCurWeap]].m_vecBorderUL = m_aMIWeaponSecondary[uYSelected * uCurWidth + uXSelected].m_vecBorderUL;
			m_aMIWeaponSecondary[CPauseScreen_auSelectedIdx[uCurWeap]].m_vecBorderRect = m_aMIWeaponSecondary[uYSelected * uCurWidth + uXSelected].m_vecBorderRect;
			m_aMIWeaponSecondary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageOfs[0] = m_aMIWeaponSecondary[uYSelected * uCurWidth + uXSelected].m_avecImageOfs[0];
			m_aMIWeaponSecondary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageRect[0] = m_aMIWeaponSecondary[uYSelected * uCurWidth + uXSelected].m_avecImageRect[0];
			m_aMIWeaponSecondary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageOfs[1] = m_aMIWeaponSecondary[uYSelected * uCurWidth + uXSelected].m_avecImageOfs[1];
			m_aMIWeaponSecondary[CPauseScreen_auSelectedIdx[uCurWeap]].m_avecImageRect[1] = m_aMIWeaponSecondary[uYSelected * uCurWidth + uXSelected].m_avecImageRect[1];
		}
	}
}

// =============================================================================================================

void CPauseScreen::_SetTextItemDraw(u32 uWhichWeapon, BOOL bDrawText)
{
	u32 i, nX, nY;
	CMenuItem *pMenuItemArray;

	pMenuItemArray = uWhichWeapon ? m_aMIWeaponSecondary : m_aMIWeaponPrimary;

	for( i=0; i<12; ++i ) {
		nX = ((CItemInst *)pMenuItemArray[i].m_pvUserData)->m_pItemData->m_uInventoryPos_x;
		nY = ((CItemInst *)pMenuItemArray[i].m_pvUserData)->m_pItemData->m_uInventoryPos_y;

		if( (nX==1 || nX==2) && (nY==1 || nY==2) ) {
			pMenuItemArray[i].SetTextDraw(bDrawText);
		}
	}
}

// =============================================================================================================

void CPauseScreen::DrawScreenTitle()
{
	CMenuScreen *pMS = m_MenuMgr.m_pCurMS;
	FASSERT(pMS != NULL);
	ftext_PrintString(m_hScreenTitle, pMS->m_wszTitle);
}

// =============================================================================================================
