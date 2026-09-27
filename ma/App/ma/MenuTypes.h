//////////////////////////////////////////////////////////////////////////////////////
// MenuTypes.h - Menu classes for Mettle Arms.
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
// 02/19/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MENUTYPES_H_
#define _MENUTYPES_H_ 1

#include "fang.h"
#include "fmath.h"
#include "ftex.h"
#include "ftext.h"
#include "fpad.h"
#include "fdraw.h"
#include "fsndfx.h"

// =============================================================================================================

enum
{
	MMINPUT_DPADUP			=	1 << 0,
	MMINPUT_DPADDOWN		=	1 << 1,
	MMINPUT_DPADLEFT		=	1 << 2,
	MMINPUT_DPADRIGHT		=	1 << 3,
	MMINPUT_LANALOGUP		=	1 << 4,
	MMINPUT_LANALOGDOWN		=	1 << 5,
	MMINPUT_LANALOGLEFT		=	1 << 6,
	MMINPUT_LANALOGRIGHT	=	1 << 7,
	MMINPUT_RANALOGUP		=	1 << 8,
	MMINPUT_RANALOGDOWN		=	1 << 9,
	MMINPUT_RANALOGLEFT		=	1 << 10,
	MMINPUT_RANALOGRIGHT	=	1 << 11,
	MMINPUT_TOPBUTTON		=	1 << 12,
	MMINPUT_RIGHTBUTTON		=	1 << 13,
	MMINPUT_BOTTOMBUTTON	=	1 << 14,
	MMINPUT_LEFTBUTTON		=	1 << 15,
	MMINPUT_LEFTSHOULDER	=	1 << 16,
	MMINPUT_RIGHTSHOULDER	=	1 << 17,

	MMINPUT_MOVEUP			=	MMINPUT_DPADUP | MMINPUT_LANALOGUP,
	MMINPUT_MOVEDOWN		=	MMINPUT_DPADDOWN | MMINPUT_LANALOGDOWN,
	MMINPUT_MOVELEFT		=	MMINPUT_DPADLEFT | MMINPUT_LANALOGLEFT,
	MMINPUT_MOVERIGHT		=	MMINPUT_DPADRIGHT | MMINPUT_LANALOGRIGHT,

	MMINPUT_DPADBUTTONS		=	MMINPUT_DPADUP | MMINPUT_DPADDOWN | MMINPUT_DPADLEFT | MMINPUT_DPADRIGHT,
	MMINPUT_LANALOGS		=	MMINPUT_LANALOGUP | MMINPUT_LANALOGDOWN | MMINPUT_LANALOGLEFT | MMINPUT_LANALOGRIGHT,
	MMINPUT_RANALOGS		=	MMINPUT_RANALOGUP | MMINPUT_RANALOGDOWN | MMINPUT_RANALOGLEFT | MMINPUT_RANALOGRIGHT,

	MMINPUT_NONE			=	0
};

// =============================================================================================================

enum MenuItemDirection_e
{
	MIDIR_UP = 0,
	MIDIR_DOWN = 1,
	MIDIR_LEFT = 2,
	MIDIR_RIGHT = 3
};

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

enum MenuItemCallbackReason_e
{
	MIREASON_BUTTONPRESSED,
	MIREASON_SELECTED,
	MIREASON_UNSELECTED
};

// =============================================================================================================

class CMenuItem;
typedef void CMenuItemCallback_t(MenuItemCallbackReason_e eReason, CMenuItem *pMI, u32 uData);

// =============================================================================================================

class CItemInst;

// =============================================================================================================

enum MenuItemState_e
{
	MISTATE_ACTIVE
};

// =============================================================================================================

const s32 CMenuItem_uMaxImages = 3;
const s32 CMenuItem_uMaxTextRegions = 2;

class CMenuItem
{
public:
	CMenuItem();
	~CMenuItem();

	void Init();
	void InitWithItemInst(CItemInst *pII);
	void Uninit();
	void SetText(u32 uTextRgnNum, cwchar *pwszNewText);				// Check to ensure that length < 40;
	void SetGraphic(u32 uImageNum, CFTexInst *ptexNew, const CFVec2 &vecST1, const CFVec2 &vecST2);
	void SetGraphic(u32 uImageNum, CFTexInst *ptexNew, const CFVec2 &vecST1, const CFVec2 &vecST2, const CFVec3 &vecOfs, const CFVec3 &vecRect);

	void Draw(const CFVec3 &vecOffset);
	void SetTextDraw(BOOL bDrawText);

	MenuItemState_e m_eState;

	CMenuItem *m_apNext[4];

	void *m_pvUserData;
	CMenuItemCallback_t *m_pfcnCallback;

	BOOL m_bDrawText;
	BOOL m_bScaleDrawnText; //Flag signifying whether drawn text (flagged with m_bDrawText) is drawn scaled or unscaled.

	// For the item as a whole.
	CFVec3 m_vecBorderUL;
	CFVec3 m_vecBorderRect;		// Height and width.

	// For each contained image.
	CFTexInst *m_pTex[CMenuItem_uMaxImages];
	CFVec3 m_avecImageOfs[CMenuItem_uMaxImages], m_avecImageRect[CMenuItem_uMaxImages];
	CFVec2 m_avecST1[CMenuItem_uMaxImages], m_avecST2[CMenuItem_uMaxImages];

	// For each contained text region.
	FTextAreaHandle_t m_ahText[CMenuItem_uMaxTextRegions];
	CFVec2 m_avecTextOfs[CMenuItem_uMaxTextRegions];	// From the UL corner of the image.  In [-1.0, 1.0], [-0.75, 0.75] coordinates.
	CFVec2 m_avecTextRect[CMenuItem_uMaxTextRegions];	// Height and width.
	wchar m_awszText[CMenuItem_uMaxTextRegions][40];

	static CFTexInst m_HudCommonTexInst;
private:

	FCLASS_STACKMEM_NOALIGN(CMenuItem);
};

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

const u32 uMaxItemsInMenuScreen = 20;

// =============================================================================================================

enum MenuScreenState_e
{
	MSSTATE_DUH
};

// =============================================================================================================

class CMenuScreen
{
public:
	CMenuScreen();
	~CMenuScreen();
	void AddMI(CMenuItem *pMI);
	void Reset();					// Sets the screen to an empty state.

	void Move(MenuItemDirection_e eDir);
	void ButtonPressed(u32 uButtonPressed);
	void SetTitleText(cwchar *pwszNewTitle);

	void Work();
	void Draw(const CFVec3 &vecOffset);

	void ForceCurrentItem( u32 nIndexToMakeCurrent );

	CMenuItem *m_pCurMI;

	CMenuScreen *m_apNext[2];
	u32 m_uMenuID;					// Set by the MenuMgr class.  Not for external use.
	u32 m_uNumMI;
	CMenuItem *m_apMI[uMaxItemsInMenuScreen];
	wchar m_wszTitle[40];

private:
	MenuScreenState_e m_eState;
//	s32 m_nCurMI;

	FCLASS_STACKMEM_NOALIGN(CMenuScreen);
};

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

const u32 CMenuMgr_uMaxScreens = 4;

// =============================================================================================================

enum MenuMgrState_e
{
	MMSTATE_STATIC,
	MMSTATE_SCROLLING,
	MMSTATE_DORMANT
};

// =============================================================================================================

class CMenuCursor
{
public:
	void AddToVtxList(FDrawVtx_t *avtxList, u32 &uCurVtx) const;

	CFVec3 m_vecCurUL, m_vecBaseRect;
	f32 m_fBPUnitOscillate, m_fThickness;

	FCLASS_STACKMEM_NOALIGN(CMenuCursor);
};

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

enum MenuCursorPosState_e
{
	MCPS_STATIONARY,
	MCPS_MOVING,
};

// =============================================================================================================

typedef enum {
	_PLATFORM_XB,
	_PLATFORM_GC,

	_PLATFORM_COUNT
} _Platform_e;

// =============================================================================================================

class CMenuMgr
{
public:
	static FSndFx_FxHandle_t m_hClickSnd;
	static FSndFx_FxHandle_t m_hSelectSnd;
	static FSndFx_FxHandle_t m_hChangeScreensSnd;
	
	CMenuScreen *m_pCurMS;

	static BOOL InitSystem();
	static void UninitSystem();

	CMenuMgr();
	~CMenuMgr();

	void Work();
	void SetDrawCursor(BOOL bDrawCursor);
	void Draw();
	void SetDormant(BOOL bEnterDormant);
	MenuMgrState_e GetState();
	void SetCurrent(CMenuScreen *pNewCurMS);
	void Disappear();
	void GetControls();

	void ScrollLeft();
	void ScrollRight();
	void UpdateCursorToCurrent();

	// Out here temporarily so that I can hack at them.
	CFVec3 m_vecCursorCurPos;
	CFVec3 m_vecCursorStartPos;
	CFVec3 m_vecCursorEndPos;
	CFVec3 m_vecCursorCurRect;
	CFVec3 m_vecCursorStartRect;
	CFVec3 m_vecCursorEndRect;

private:
	static BOOL m_bIsInitialized;
		
	void ReverseScroll();
	void StartScroll(f32 fDir);			// -1.0f = left, +1.0f = right

	MenuMgrState_e m_eState, m_ePrevState;

	u32 m_uButtons, m_uButtonsLatched;

	BOOL m_bDrawCursor;

	// Related to the scrolling.
	CFVec3 m_vecCurScrollPos, m_vecStartScrollPos, m_vecEndScrollPos;
	CMenuScreen *m_pNextMS;
	f32 m_fScrollTimer;
	s32 m_nScrollsPending;

	// Related to the cursor.
	CMenuCursor m_MenuCursor;
	MenuCursorPosState_e m_eMCPState;

	f32 m_fCursorOscillateTimer, m_fCursorMoveTimer;

	FCLASS_STACKMEM_NOALIGN(CMenuMgr);
};

#endif
