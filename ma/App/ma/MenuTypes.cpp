//////////////////////////////////////////////////////////////////////////////////////
// MenuTypes.cpp - Menu classes for Mettle Arms.
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

#include "MenuTypes.h"
#include "ItemInst.h"
#include "Item.h"
#include "gamepad.h"
#include "player.h"
#include "fclib.h"
#include "floop.h"
#include "fresload.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif

// =============================================================================================================

static const f32 CMenuMgr_fCursorMoveTime = 0.10f;//2.0f;//0.12f;
static const f32 CMenuMgr_fMenuScrollTime = 0.25f;//10.0f;//0.2f;

static const f32 CMenuMgr_fOOCursorMoveTime = 1.0f / CMenuMgr_fCursorMoveTime;
static const f32 CMenuMgr_fOOMenuScrollTime = 1.0f / CMenuMgr_fMenuScrollTime;

// =============================================================================================================
CFTexInst CMenuItem::m_HudCommonTexInst;

CMenuItem::CMenuItem()
{
	m_eState = MISTATE_ACTIVE;
	m_pfcnCallback = NULL;

	m_apNext[0] = NULL;
	m_apNext[1] = NULL;
	m_apNext[2] = NULL;
	m_apNext[3] = NULL;

	u32 uCurImage;
	for(uCurImage = 0; uCurImage < CMenuItem_uMaxImages; ++uCurImage)
	{
		m_pTex[uCurImage] = NULL;
	}

	m_bDrawText = TRUE;
	m_bScaleDrawnText = TRUE;

	u32 uCurTxtRgn;
	for(uCurTxtRgn = 0; uCurTxtRgn < CMenuItem_uMaxTextRegions; ++uCurTxtRgn)
	{
		m_ahText[uCurTxtRgn] = 0;
		m_awszText[uCurTxtRgn][0] = L'\0';
	}
}

// =============================================================================================================

CMenuItem::~CMenuItem()
{
	Uninit();
}

// =============================================================================================================

void CMenuItem::Init()
{
	// Set up the text area.
	FTextArea_t oTextArea;
	ftext_SetToDefaults(&oTextArea);
	oTextArea.bVisible = FALSE;
	oTextArea.fNumberOfLines = 1.0f;
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_RIGHT;
	oTextArea.ohFont = '4';

	if(m_ahText[0] == 0)
	{
		m_ahText[0] = ftext_Create(&oTextArea);
	}
	if(m_ahText[1] == 0)
	{
		m_ahText[1] = ftext_Create(&oTextArea);
	}
}

// =============================================================================================================

void CMenuItem::InitWithItemInst(CItemInst *pII)
{
	CItem *pI = pII->m_pItemData;
	FASSERT(pI != NULL);

	m_pvUserData = (void *)(pII);

	// For below:
	// Image 0 is the icon + frame,
	// Image 1 is the ammo frame,
	// Image 2 is the EUK ticks.

	m_pTex[0] = &pII->m_pItemData->m_texIcon;
	m_pTex[1] = &m_HudCommonTexInst;
	m_pTex[2] = &m_HudCommonTexInst;
	m_avecST1[0] = pII->m_pItemData->m_vecST1;
	m_avecST2[0] = pII->m_pItemData->m_vecST2;

	switch(pI->m_uCurLevel)
	{
		case 0:
		{
			m_avecST1[2].Set(0.0f, 0.0f);
			m_avecST2[2].Set(0.0f, 0.0f);
			break;
		}
		case 1:
		{
			m_avecST1[2].Set(0.75f, 0.394f);
			m_avecST2[2].Set(0.847f, 0.5f);
			break;
		}
		case 2:
		{
			m_avecST1[2].Set(0.902f, 0.394f);
			m_avecST2[2].Set(1.0f, 0.5f);
			break;
		}
		case 3:
		{
			m_avecST1[2].Set(0.75f, 0.25f);
			m_avecST2[2].Set(0.844f, 0.351f);
			break;
		}
	}

	m_avecImageRect[0].x = m_vecBorderRect.y;
	m_avecImageRect[0].y = m_vecBorderRect.y;

	m_avecImageRect[1].Set(m_vecBorderRect.y * 0.45f * (0.238f / 0.125f), m_vecBorderRect.y * 0.45f, 0.0f);

	m_avecImageRect[2].Set(m_vecBorderRect.y * 0.40f, m_vecBorderRect.y * 0.40f, 0.0f);

	if(pI->IsSecondaryOnly())
	{
		m_avecImageOfs[0].Set(0.0f, 0.0f, 0.0f);

		m_avecImageOfs[1].Set(m_vecBorderRect.x - m_vecBorderRect.y * 0.45f * (0.238f / 0.125f), -m_vecBorderRect.y * 0.55f, 0.0f);
		m_avecST1[1].Set(0.738f, 0.25f);
		m_avecST2[1].Set(0.5f, 0.375f);

		// Swap the S's of the EUK ticks.
		f32 fTemp;
		fTemp = m_avecST1[2].x;
		m_avecST1[2].x = m_avecST2[2].x;
		m_avecST2[2].x = fTemp;

		m_avecImageOfs[2].Set(m_vecBorderRect.x - m_vecBorderRect.y * 0.40f, 0.0f, 0.0f);
	}
	else if(pI->IsPrimary())
	{
		m_avecImageOfs[0].Set(m_vecBorderRect.x - m_vecBorderRect.y, 0.0f, 0.0f);

		m_avecImageOfs[1].Set(0.0f, -m_vecBorderRect.y * 0.55f, 0.0f);
		m_avecST1[1].Set(0.5f, 0.25f);
		m_avecST2[1].Set(0.738f, 0.375f);

		m_avecImageOfs[2].Set(0.0f, 0.0f, 0.0f);
	}
	else	// It must be an item.
	{
		m_bScaleDrawnText = FALSE; // we don't want to scale the item counter text for visual purposes...
		m_avecImageOfs[0].Set(m_vecBorderRect.x - m_vecBorderRect.y, 0.0f, 0.0f);

		m_avecImageRect[1].Set(m_vecBorderRect.y * 0.45f * (0.238f / 0.125f), m_vecBorderRect.y * 0.30f, 0.0f);
		m_avecImageOfs[1].Set(0.0f, -m_vecBorderRect.y * 0.70f, 0.0f);
		m_avecST1[1].Set(0.500f, 0.379f + 0.03125f);
		m_avecST2[1].Set(0.738f, 0.493f);

		m_avecImageOfs[2].Set(0.0f, 0.0f, 0.0f);
	}
	// Text region 0 is the top box,
	// Text region 1 is the bottom box.
	m_avecTextOfs[0].Set(0.01f, -m_vecBorderRect.y * 0.5f - 0.02f);
	m_avecTextRect[0].Set(m_vecBorderRect.y * 0.31f * 1.20f, m_vecBorderRect.y * 0.31f * 0.5f);

	// Switch the sides if we're a secondary.
	if(pI->IsSecondaryOnly())
	{
		m_avecTextOfs[0].x = m_vecBorderRect.x - m_avecTextRect[0].x - m_avecTextOfs[0].x;
	}

	if(!pI->IsPrimary() && !pI->IsSecondary())
	{
		m_avecTextOfs[0].x -= 0.01f;
		m_avecTextOfs[0].y -= 0.0375f; // -= 0.04f;
		m_avecTextRect[0].x += 0.05f;
	}

	m_avecTextOfs[1].Set(0.0f, -0.005f);
	m_avecTextOfs[1] += m_avecTextOfs[0];
	m_avecTextOfs[1].y -= m_avecTextRect[0].y;
	m_avecTextRect[1] = m_avecTextRect[0];

	// Reinit the text regions here.

	FASSERT(m_ahText[0] != 0);
	FASSERT(m_ahText[1] != 0);

	// JUSTIN: This should probably be getting set blah blah.
/*	if(pII->m_nReserveAmmo != CItemInst_nNoAmmo)
	{
		char szTemp[4];
		sprintf(szTemp, "%d", pII->m_nClipAmmo);
		fclib_strcpy(m_aszText[0], szTemp);
		if(pII->m_nReserveAmmo != CItemInst_nNoMaxAmmo)
		{
			sprintf(szTemp, "%d", pII->m_nReserveAmmo);
			fclib_strcpy(m_aszText[1], szTemp);
		}
	}*/
	pII->GetItemText(m_awszText[0], m_awszText[1], 40);
/*
	if(pII->m_uMaxAmmo != 0)
	{
		char szTemp[4];
		sprintf(szTemp, "%d", pII->m_uCurAmmo);
		fclib_strcpy(m_aszText[0], szTemp);
		sprintf(szTemp, "%d", pII->m_uMaxAmmo);
		fclib_strcpy(m_aszText[1], szTemp);
	}*/
}

// =============================================================================================================

void CMenuItem::Uninit()
{
	m_pfcnCallback = NULL;

	m_pTex[0] = NULL;
	m_pTex[1] = NULL;
	m_pTex[2] = NULL;

	m_apNext[0] = NULL;
	m_apNext[1] = NULL;
	m_apNext[2] = NULL;
	m_apNext[3] = NULL;

	u32 uCurImage;
	for(uCurImage = 0; uCurImage < CMenuItem_uMaxImages; ++uCurImage)
	{
		m_pTex[uCurImage] = NULL;
	}

	u32 uCurTxtRgn;
	for(uCurTxtRgn = 0; uCurTxtRgn < CMenuItem_uMaxTextRegions; ++uCurTxtRgn)
	{
		m_ahText[uCurTxtRgn] = 0;
		m_awszText[uCurTxtRgn][0] = L'\0';
	}
}

// =============================================================================================================

void CMenuItem::SetText(u32 uTextRgnNum, cwchar *pwszNewText)
{
	FASSERT(fclib_wcslen(pwszNewText) < 40);
	fclib_wcscpy(m_awszText[uTextRgnNum], pwszNewText);
}

// =============================================================================================================

void CMenuItem::SetGraphic(u32 uImageNum, CFTexInst *ptexNew, const CFVec2 &vecST1, const CFVec2 &vecST2)
{
	m_pTex[uImageNum] = ptexNew;
	m_avecST1[uImageNum] = vecST1;
	m_avecST2[uImageNum] = vecST2;
}

// =============================================================================================================

void CMenuItem::SetGraphic(u32 uImageNum, CFTexInst *ptexNew, const CFVec2 &vecST1, const CFVec2 &vecST2, const CFVec3 &vecOfs, const CFVec3 &vecRect)
{
	m_pTex[uImageNum] = ptexNew;
	m_avecST1[uImageNum] = vecST1;
	m_avecST2[uImageNum] = vecST2;
	m_avecImageOfs[uImageNum] = vecOfs;
	m_avecImageRect[uImageNum] = vecRect;
}

// =============================================================================================================

// This actually *draws* the menu item and all associated elements.
void CMenuItem::Draw(const CFVec3 &vecOffset)
{
	FDrawVtx_t avtxTemp[4];
	u32 uCurVtx;

	f32 fBorderX1 = m_vecBorderUL.x + vecOffset.x;
	f32 fBorderX2 = fBorderX1 + m_vecBorderRect.x;
	f32 fBorderY1 = m_vecBorderUL.y + vecOffset.y;
	f32 fBorderY2 = fBorderY1 - m_vecBorderRect.y;

	// Draw each of the images.
	u32 uCurImage;
	for(uCurImage = 0; uCurImage < CMenuItem_uMaxImages; ++uCurImage)
	{
		if( m_pTex[uCurImage] && m_pTex[uCurImage]->GetTexDef() != NULL)
		{
			fdraw_SetTexture(m_pTex[uCurImage]);

			uCurVtx = 0;
			f32 fAlpha = 1.0f;

			// All coordinates are relative to the border.
			f32 fX1 = fBorderX1 + m_avecImageOfs[uCurImage].x;
			f32 fX2 = fX1 + m_avecImageRect[uCurImage].x;
			f32 fY1 = fBorderY1 + m_avecImageOfs[uCurImage].y;
			f32 fY2 = fY1 - m_avecImageRect[uCurImage].y;

			f32 fS1 = m_avecST1[uCurImage].x;
			f32 fS2 = m_avecST2[uCurImage].x;
			f32 fT1 = m_avecST1[uCurImage].y;
			f32 fT2 = m_avecST2[uCurImage].y;

			avtxTemp[uCurVtx].ColorRGBA.White();
			avtxTemp[uCurVtx].ColorRGBA.fAlpha = fAlpha;
			avtxTemp[uCurVtx].Pos_MS.Set(fX1, fY1, 0.0f);
			avtxTemp[uCurVtx].ST.Set(fS1, fT1);

			avtxTemp[++uCurVtx].ColorRGBA.White();
			avtxTemp[uCurVtx].ColorRGBA.fAlpha = fAlpha;
			avtxTemp[uCurVtx].Pos_MS.Set(fX2, fY1, 0.0f);
			avtxTemp[uCurVtx].ST.Set(fS2, fT1);

			avtxTemp[++uCurVtx].ColorRGBA.White();
			avtxTemp[uCurVtx].ColorRGBA.fAlpha = fAlpha;
			avtxTemp[uCurVtx].Pos_MS.Set(fX1, fY2, 0.0f);
			avtxTemp[uCurVtx].ST.Set(fS1, fT2);

			avtxTemp[++uCurVtx].ColorRGBA.White();
			avtxTemp[uCurVtx].ColorRGBA.fAlpha = fAlpha;
			avtxTemp[uCurVtx].Pos_MS.Set(fX2, fY2, 0.0f);
			avtxTemp[uCurVtx].ST.Set(fS2, fT2);

			++uCurVtx;

			fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, avtxTemp, uCurVtx);
		}
	}

	if(m_bDrawText)
	{
		u32 uCurTxtRgn;
		for(uCurTxtRgn = 0; uCurTxtRgn < CMenuItem_uMaxTextRegions; ++uCurTxtRgn)
		{
			if(m_awszText[uCurTxtRgn][0] != L'\0')
			{
				f32 fTextX1 = fBorderX1 + m_avecTextOfs[uCurTxtRgn].x;
				f32 fTextX2 = fTextX1 + m_avecTextRect[uCurTxtRgn].x;
				f32 fTextY1 = fBorderY1 + m_avecTextOfs[uCurTxtRgn].y;
				f32 fTextY2 = fTextY1 - m_avecTextRect[uCurTxtRgn].y;

				fTextX1 = (1.0f + fTextX1) * 0.5f;
				fTextX2 = (1.0f + fTextX2) * 0.5f;
				fTextY1 = (0.75f - fTextY1) * 0.5f;
				fTextY2 = (0.75f - fTextY2) * 0.5f;

				ftext_GetAttributes(m_ahText[uCurTxtRgn])->fUpperLeftX = fTextX1;
				ftext_GetAttributes(m_ahText[uCurTxtRgn])->fUpperLeftY = fTextY1;
				ftext_GetAttributes(m_ahText[uCurTxtRgn])->fLowerRightX = fTextX2;
				ftext_GetAttributes(m_ahText[uCurTxtRgn])->fLowerRightY = fTextY2;

				//ftext_PrintString(m_ahText[uCurTxtRgn], m_awszText[uCurTxtRgn]);
				// RAF -- Modifying to make sure that text prints out non-scaled...
				ftext_Printf(m_ahText[uCurTxtRgn], L"~o%d%ls", !m_bScaleDrawnText, m_awszText[uCurTxtRgn]);
			}
		}
	}
}

// =============================================================================================================

void CMenuItem::SetTextDraw(BOOL bDrawText)
{
	m_bDrawText = bDrawText;
}

// =============================================================================================================

// =============================================================================================================
// =============================================================================================================

CMenuScreen::CMenuScreen()
{
	m_uNumMI = 0;

	m_apNext[0] = NULL;
	m_apNext[1] = NULL;

	m_pCurMI = NULL;

	m_uMenuID = 9873;
}

// =============================================================================================================

CMenuScreen::~CMenuScreen()
{
	m_uNumMI = 0;

	m_apNext[0] = NULL;
	m_apNext[1] = NULL;

	m_pCurMI = NULL;

	m_uMenuID = 9873;
}

// =============================================================================================================

void CMenuScreen::AddMI(CMenuItem *pMI)
{
	FASSERT(m_uNumMI != uMaxItemsInMenuScreen);
	FASSERT(pMI != NULL);

	m_apMI[m_uNumMI] = pMI;
	++m_uNumMI;

	//// If this is the first MI, set it to be the current one.
	//
	if(m_pCurMI == NULL)
		m_pCurMI = m_apMI[0];
	//
	////
}

// =============================================================================================================

void CMenuScreen::Reset()
{
	m_uNumMI = 0;

	m_pCurMI = NULL;
}

// =============================================================================================================

void CMenuScreen::Move(enum MenuItemDirection_e eDir)
{
	CMenuItem *pMINext = m_pCurMI->m_apNext[eDir];
	if(pMINext != NULL)
	{
		// TODO : Add notify to old MI.
		m_pCurMI = m_pCurMI->m_apNext[eDir];

		fsndfx_Play2D( CMenuMgr::m_hClickSnd );

		// TODO : Add notify to new MI.
	}
}

void CMenuScreen::ForceCurrentItem( u32 nIndexToMakeCurrent ) {
	FASSERT( nIndexToMakeCurrent < m_uNumMI );

	m_pCurMI = m_apMI[nIndexToMakeCurrent];
}

// =============================================================================================================

void CMenuScreen::ButtonPressed(u32 uButtonPressed)
{
	if(m_pCurMI->m_pfcnCallback != NULL)
		m_pCurMI->m_pfcnCallback(MIREASON_BUTTONPRESSED, m_pCurMI, uButtonPressed);
}

// =============================================================================================================

void CMenuScreen::SetTitleText(cwchar *pwszNewTitle)
{
	FASSERT(fclib_wcslen(pwszNewTitle) < 40);
	fclib_wcscpy(m_wszTitle, pwszNewTitle);
}

// =============================================================================================================

void CMenuScreen::Work()
{
}

// =============================================================================================================

void CMenuScreen::Draw(const CFVec3 &vecOffset)
{
	FDrawVtx_t avtxItems[6 * uMaxItemsInMenuScreen];
	u32 uCurVtx = 0;

	u32 uCurMIIdx;
	for(uCurMIIdx = 0; uCurMIIdx < m_uNumMI; ++uCurMIIdx)
	{
		m_apMI[uCurMIIdx]->Draw(vecOffset);
	}
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

void CMenuCursor::AddToVtxList(FDrawVtx_t *avtxList, u32 &uCurVtx) const
{
	f32 fX1I = m_vecCurUL.x - 0.03f;
	f32 fX2I = m_vecCurUL.x + m_vecBaseRect.x + 0.02f;
	f32 fY1I = m_vecCurUL.y + 0.005f;
	f32 fY2I = m_vecCurUL.y - m_vecBaseRect.y - 0.005f;

	f32 fX1O = fX1I - m_fThickness;
	f32 fX2O = fX2I + m_fThickness;
	f32 fY1O = fY1I + m_fThickness;
	f32 fY2O = fY2I - m_fThickness;

//	CFColorRGBA rgbaCursor(236.0f / 256.0f/*0.8f*/, 164.0f / 256.0f/*0.4f*/, 0.0f, 1.0f/*0.8f*/);
	CFColorRGBA rgbaCursor( 252.0f/255.0f, 235.0f/255.0f, 40.0f/255.0f );

	avtxList[uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX2O, fY1O, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX2I, fY1I, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX1O, fY1O, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX1I, fY1I, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX1O, fY2O, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX1I, fY2I, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX2O, fY2O, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX2I, fY2I, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX2O, fY1O, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	avtxList[++uCurVtx].ColorRGBA = rgbaCursor;//.OpaqueRed();
	avtxList[uCurVtx].Pos_MS.Set(fX2I, fY1I, 0.0f);
	avtxList[uCurVtx].ST.Set(0.0f, 0.0f);

	++uCurVtx;

}

BOOL CMenuMgr::m_bIsInitialized = FALSE;
FSndFx_FxHandle_t CMenuMgr::m_hClickSnd;
FSndFx_FxHandle_t CMenuMgr::m_hSelectSnd;
FSndFx_FxHandle_t CMenuMgr::m_hChangeScreensSnd;

BOOL CMenuMgr::InitSystem()
{
	FASSERT(!m_bIsInitialized);

	// load the GUI bank, it is always loaded
	fresload_Load( FSNDFX_RESTYPE, "GUI" );

	// grab handles to a few
	m_hClickSnd = fsndfx_GetFxHandle( "HUD Click" );
	m_hSelectSnd = fsndfx_GetFxHandle( "HUD Select" );
	m_hChangeScreensSnd = fsndfx_GetFxHandle( "VolumeTest" );
	
	m_bIsInitialized = TRUE;

	return(TRUE);
}

// =============================================================================================================

void CMenuMgr::UninitSystem()
{
	m_bIsInitialized = FALSE;
}

// =============================================================================================================

CMenuMgr::CMenuMgr()
{
	m_pCurMS = NULL;
	m_pNextMS = NULL;

	m_uButtons = MMINPUT_NONE;
	m_uButtonsLatched = 0;

	m_MenuCursor.m_fThickness = 0.01f;

	m_eMCPState = MCPS_STATIONARY;

	m_bDrawCursor = TRUE;

	m_nScrollsPending = 0;

	m_vecCurScrollPos.Set(0.0f, 0.0f, 0.0f);
}

// =============================================================================================================

CMenuMgr::~CMenuMgr()
{
	m_pCurMS = NULL;
	m_pNextMS = NULL;
}

// =============================================================================================================

void CMenuMgr::Work()
{
	FASSERT(m_bIsInitialized);

	GetControls();

	if(m_eState == MMSTATE_DORMANT)
		return;

	//// State transitions.
	//
	//// Check for scroll requests.
	//
	if(m_eMCPState == MCPS_STATIONARY)
	{
		if((m_uButtonsLatched & MMINPUT_LEFTSHOULDER) != MMINPUT_NONE)
		{
			fsndfx_Play2D( CMenuMgr::m_hChangeScreensSnd );
			--m_nScrollsPending;
			if(m_nScrollsPending > 0)
			{
				// It's a turnaround.  Reverse the scroll that is currently happening.
				ReverseScroll();
			}
		}
		else if((m_uButtonsLatched & MMINPUT_RIGHTSHOULDER) != MMINPUT_NONE)
		{
			fsndfx_Play2D( CMenuMgr::m_hChangeScreensSnd );
			++m_nScrollsPending;
			if(m_nScrollsPending < 0)
			{
				// It's a turnaround.  Reverse the scroll that is currently happening.
				ReverseScroll();
			}
		}
	}
	//
	////

	//// Handle any pending scrolls (these take precendence over cursor movements).
	//
	if(m_eState == MMSTATE_STATIC)
	{
		if(m_nScrollsPending > 0)
		{
			// Scrolling to the right.
			StartScroll(+1.0f);
			--m_nScrollsPending;
		}
		else if(m_nScrollsPending < 0)
		{
			// Scrolling to the left.
			StartScroll(-1.0f);
			++m_nScrollsPending;
		}
	}
	//
	////

	//// Check if they've tried to move the cursor or pressed a button.
	//
	switch(m_eState)
	{
		case MMSTATE_STATIC:
		{
			if((m_uButtonsLatched & MMINPUT_MOVELEFT) != MMINPUT_NONE)
			{
				m_pCurMS->Move(MIDIR_LEFT);
				m_vecCursorStartPos = m_vecCursorCurPos;
//					m_vecCursorEndPos = m_pCurMS->m_pCurMI->m_vecUL;
				m_vecCursorEndPos = m_pCurMS->m_pCurMI->m_vecBorderUL;
				m_vecCursorStartRect = m_vecCursorCurRect;
				m_vecCursorEndRect = m_pCurMS->m_pCurMI->m_vecBorderRect;
				m_fCursorMoveTimer = 0.0f;
				m_eMCPState = MCPS_MOVING;
			}
			else if((m_uButtonsLatched & MMINPUT_MOVERIGHT) != MMINPUT_NONE)
			{
				m_pCurMS->Move(MIDIR_RIGHT);
				m_vecCursorStartPos = m_vecCursorCurPos;
				m_vecCursorEndPos = m_pCurMS->m_pCurMI->m_vecBorderUL;
				m_vecCursorStartRect = m_vecCursorCurRect;
				m_vecCursorEndRect = m_pCurMS->m_pCurMI->m_vecBorderRect;
				m_fCursorMoveTimer = 0.0f;
				m_eMCPState = MCPS_MOVING;
			}

			if((m_uButtonsLatched & MMINPUT_MOVEUP) != MMINPUT_NONE)
			{
				m_pCurMS->Move(MIDIR_UP);
				m_vecCursorStartPos = m_vecCursorCurPos;
				m_vecCursorEndPos = m_pCurMS->m_pCurMI->m_vecBorderUL;
				m_vecCursorStartRect = m_vecCursorCurRect;
				m_vecCursorEndRect = m_pCurMS->m_pCurMI->m_vecBorderRect;
				m_fCursorMoveTimer = 0.0f;
				m_eMCPState = MCPS_MOVING;
			}
			else if((m_uButtonsLatched & MMINPUT_MOVEDOWN) != MMINPUT_NONE)
			{
				m_pCurMS->Move(MIDIR_DOWN);
				m_vecCursorStartPos = m_vecCursorCurPos;
				m_vecCursorEndPos = m_pCurMS->m_pCurMI->m_vecBorderUL;
				m_vecCursorStartRect = m_vecCursorCurRect;
				m_vecCursorEndRect = m_pCurMS->m_pCurMI->m_vecBorderRect;
				m_fCursorMoveTimer = 0.0f;
				m_eMCPState = MCPS_MOVING;
			}

			if((m_uButtonsLatched & MMINPUT_TOPBUTTON) != MMINPUT_NONE)
			{
				m_pCurMS->ButtonPressed(MMINPUT_TOPBUTTON);
			}

			if((m_uButtonsLatched & MMINPUT_RIGHTBUTTON) != MMINPUT_NONE)
			{
				m_pCurMS->ButtonPressed(MMINPUT_RIGHTBUTTON);
			}

			if((m_uButtonsLatched & MMINPUT_BOTTOMBUTTON) != MMINPUT_NONE)
			{
				m_pCurMS->ButtonPressed(MMINPUT_BOTTOMBUTTON);
			}

			if((m_uButtonsLatched & MMINPUT_LEFTBUTTON) != MMINPUT_NONE)
			{
				m_pCurMS->ButtonPressed(MMINPUT_LEFTBUTTON);
			}

			break;
		}
		case MMSTATE_SCROLLING:
		{
			m_fScrollTimer += FLoop_fRealPreviousLoopSecs;
			if(m_fScrollTimer >= CMenuMgr_fMenuScrollTime)
			{
				m_vecCurScrollPos.Set(0.0f, 0.0f, 0.0f);
				m_vecCursorCurPos = m_vecCursorEndPos;
				m_vecCursorCurRect = m_vecCursorEndRect;

				m_pCurMS = m_pNextMS;
				m_eState = MMSTATE_STATIC;
			}
			break;
		}
	}
	//
	////

	//// Do cursor state transitions.
	//
	switch(m_eMCPState)
	{
		case MCPS_STATIONARY:
		{
			break;
		}
		case MCPS_MOVING:
		{
			m_fCursorMoveTimer += FLoop_fRealPreviousLoopSecs;
			if(m_fCursorMoveTimer >= CMenuMgr_fCursorMoveTime)
			{
				m_vecCursorCurPos = m_vecCursorEndPos;
				m_vecCursorCurRect = m_vecCursorEndRect;

				m_eMCPState = MCPS_STATIONARY;
			}
			break;
		}
	}
	//
	////
	//
	//// State transitions.

	//// Do normal cursor work.
	//
	switch(m_eMCPState)
	{
		case MCPS_STATIONARY:
		{
			break;
		}
		case MCPS_MOVING:
		{
			//// Determine the new position for the cursor.
			//
			CFVec3 vecTemp;
			f32 fUnitTime = m_fCursorMoveTimer * CMenuMgr_fOOCursorMoveTime;
			f32 fWeight1 = 1.0f - fUnitTime;
			f32 fWeight2 = fUnitTime;

			m_vecCursorCurPos = m_vecCursorStartPos;
			m_vecCursorCurPos *= fWeight1;
			vecTemp = m_vecCursorEndPos;
			vecTemp *= fWeight2;
			m_vecCursorCurPos += vecTemp;

			m_vecCursorCurRect = m_vecCursorStartRect;
			m_vecCursorCurRect *= fWeight1;
			vecTemp = m_vecCursorEndRect;
			vecTemp *= fWeight2;
			m_vecCursorCurRect += vecTemp;
			//
			////

			break;
		}
	}
	//
	////

	////
	//
	switch(m_eState)
	{
		case MMSTATE_STATIC:
		{
			break;
		}
		case MMSTATE_SCROLLING:
		{
			// In this case we need to get a new position for the menu screens, *and* for the cursor.
			CFVec3 vecTemp;
			f32 fUnitTime = m_fScrollTimer * CMenuMgr_fOOMenuScrollTime;
			f32 fWeight1 = 1.0f - fUnitTime;
			f32 fWeight2 = fUnitTime;

			m_vecCursorCurPos = m_vecCursorStartPos;
			m_vecCursorCurPos *= fWeight1;
			vecTemp = m_vecCursorEndPos;
			vecTemp *= fWeight2;
			m_vecCursorCurPos += vecTemp;

			m_vecCursorCurRect = m_vecCursorStartRect;
			m_vecCursorCurRect *= fWeight1;
			vecTemp = m_vecCursorEndRect;
			vecTemp *= fWeight2;
			m_vecCursorCurRect += vecTemp;

			m_vecCurScrollPos = m_vecStartScrollPos;
			m_vecCurScrollPos *= fWeight1;
			vecTemp = m_vecEndScrollPos;
			vecTemp *= fWeight2;
			m_vecCurScrollPos += vecTemp;

			break;
		}
	}
	//
	////

	//// Set up the cursor.
	//
	m_MenuCursor.m_vecCurUL = m_vecCursorCurPos;
	m_MenuCursor.m_vecBaseRect = m_vecCursorCurRect;
	//
	////
}

// =============================================================================================================

void CMenuMgr::SetDrawCursor(BOOL bDrawCursor)
{
	FASSERT(m_bIsInitialized);

	m_bDrawCursor = bDrawCursor;
}

// =============================================================================================================

void CMenuMgr::Draw()
{
	FASSERT(m_bIsInitialized);

	// Assume FDRAW_COLORFUNC_DECALTEX_AT coming in.
	m_pCurMS->Draw(m_vecCurScrollPos);
	if(m_eState == MMSTATE_SCROLLING)
	{
		CFVec3 vecTemp = m_vecCurScrollPos - m_vecEndScrollPos;
		m_pNextMS->Draw(vecTemp);
	}

	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);

	FDrawVtx_t avtxCursor[10];
	u32 uCurVtx = 0;

	m_MenuCursor.AddToVtxList(avtxCursor, uCurVtx);
	FASSERT(uCurVtx <= 10);

	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, avtxCursor, uCurVtx);
}

// =============================================================================================================

void CMenuMgr::SetDormant(BOOL bEnterDormant)
{
	FASSERT(m_bIsInitialized);

	if(bEnterDormant)
	{
		m_ePrevState = m_eState;
		m_eState = MMSTATE_DORMANT;
	}
	else
	{
		FASSERT(m_eState == MMSTATE_DORMANT);
		m_eState = m_ePrevState;
	}
}

// =============================================================================================================

MenuMgrState_e CMenuMgr::GetState()
{
	FASSERT(m_bIsInitialized);

	return(m_eState);
}

// =============================================================================================================

void CMenuMgr::SetCurrent(CMenuScreen *pNewCurMS)
{
	FASSERT(m_bIsInitialized);

	m_pCurMS = pNewCurMS;
	m_vecCursorCurPos = pNewCurMS->m_pCurMI->m_vecBorderUL;
	m_vecCursorCurRect = pNewCurMS->m_pCurMI->m_vecBorderRect;
}

void CMenuMgr::ScrollLeft()
{
	if(m_eMCPState == MCPS_STATIONARY)
	{
		fsndfx_Play2D( CMenuMgr::m_hChangeScreensSnd );
		--m_nScrollsPending;
		if(m_nScrollsPending > 0)
		{
			ReverseScroll();
		}
	}
}

void CMenuMgr::ScrollRight()
{
	if(m_eMCPState == MCPS_STATIONARY)
	{
		fsndfx_Play2D( CMenuMgr::m_hChangeScreensSnd );
		++m_nScrollsPending;
		if(m_nScrollsPending < 0)
		{
			ReverseScroll();
		}
	}
}

void CMenuMgr::UpdateCursorToCurrent()
{
	if( m_pCurMS && m_pCurMS->m_pCurMI )
	{
		m_vecCursorCurPos = m_pCurMS->m_pCurMI->m_vecBorderUL;
		m_vecCursorCurRect = m_pCurMS->m_pCurMI->m_vecBorderRect;
		m_vecCursorStartPos = m_vecCursorCurPos;
		m_vecCursorEndPos = m_vecCursorCurPos;
		m_fCursorMoveTimer = 0.0f;
	}
}

// =============================================================================================================

void CMenuMgr::GetControls()
{
	FASSERT(m_bIsInitialized);

	u32 uLastButtons = m_uButtons;

	m_uButtons = 0;

	// set the controller port to the menu
	u32 nControllerPort = CPlayer::m_pCurrent->m_nControllerIndex;
	GamepadMap_e nLastMapping = gamepad_GetMapping( nControllerPort );
	gamepad_SetMapping( nControllerPort, GAMEPAD_MAP_MENU );

	// D-Pad:
	if(FMATH_FABS(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_DPAD_Y]->fCurrentState) > FMATH_FABS(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_DPAD_X]->fCurrentState))
	{
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_DPAD_Y]->fCurrentState > 0.0f)
			m_uButtons |= MMINPUT_DPADUP;
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_DPAD_Y]->fCurrentState < 0.0f)
			m_uButtons |= MMINPUT_DPADDOWN;
	}
	else
	{
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_DPAD_X]->fCurrentState > 0.0f)
			m_uButtons |= MMINPUT_DPADRIGHT;
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_DPAD_X]->fCurrentState < 0.0f)
			m_uButtons |= MMINPUT_DPADLEFT;
	}

	// Left analog:
	if(FMATH_FABS(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_LEFT_ANALOG_Y]->fCurrentState) > FMATH_FABS(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_LEFT_ANALOG_X]->fCurrentState))
	{
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_LEFT_ANALOG_Y]->fCurrentState > 0.0f)
			m_uButtons |= MMINPUT_LANALOGUP;
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_LEFT_ANALOG_Y]->fCurrentState < 0.0f)
			m_uButtons |= MMINPUT_LANALOGDOWN;
	}
	else
	{
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_LEFT_ANALOG_X]->fCurrentState > 0.0f)
			m_uButtons |= MMINPUT_LANALOGRIGHT;
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_LEFT_ANALOG_X]->fCurrentState < 0.0f)
			m_uButtons |= MMINPUT_LANALOGLEFT;
	}

	// Right analog:
	if(FMATH_FABS(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_RIGHT_ANALOG_Y]->fCurrentState) > FMATH_FABS(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_RIGHT_ANALOG_X]->fCurrentState))
	{
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_RIGHT_ANALOG_Y]->fCurrentState > 0.0f)
			m_uButtons |= MMINPUT_RANALOGUP;
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_RIGHT_ANALOG_Y]->fCurrentState < 0.0f)
			m_uButtons |= MMINPUT_RANALOGDOWN;
	}
	else
	{
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_RIGHT_ANALOG_X]->fCurrentState > 0.0f)
			m_uButtons |= MMINPUT_RANALOGRIGHT;
		if(Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_RIGHT_ANALOG_X]->fCurrentState < 0.0f)
			m_uButtons |= MMINPUT_RANALOGLEFT;
	}

	if((Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_LEFT_SHOULDER]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= MMINPUT_LEFTSHOULDER;
	if((Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_RIGHT_SHOULDER]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= MMINPUT_RIGHTSHOULDER;
#if defined(MA_PC_INPUT)
	if( nControllerPort == pcinput_KeyboardPort() && !pcinput_IsTextInput() )
	{
		// the pause menu's pages: Q / E, or Tab / Shift+Tab
		if( pcinput_KeyHeld( 'Q' ) )
			m_uButtons |= MMINPUT_LEFTSHOULDER;
		if( pcinput_KeyHeld( 'E' ) )
			m_uButtons |= MMINPUT_RIGHTSHOULDER;
		if( pcinput_KeyHeld( VK_TAB ) )
		{
			if( pcinput_KeyHeld( VK_SHIFT ) )
				m_uButtons |= MMINPUT_LEFTSHOULDER;
			else
				m_uButtons |= MMINPUT_RIGHTSHOULDER;
		}
	}
#endif

	if((Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_BUTTON_TOP]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= MMINPUT_TOPBUTTON;
	if((Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_BACK]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= MMINPUT_RIGHTBUTTON;
	if((Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_ACCEPT]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= MMINPUT_BOTTOMBUTTON;
	if((Gamepad_aapSample[nControllerPort][GAMEPAD_MENU_BUTTON_SIDE]->uLatches & FPAD_LATCH_ON) == FPAD_LATCH_ON)
		m_uButtons |= MMINPUT_LEFTBUTTON;

	m_uButtonsLatched = m_uButtons & (~uLastButtons);
	if((uLastButtons & MMINPUT_DPADBUTTONS) != MMINPUT_NONE)
		m_uButtonsLatched &= (~MMINPUT_DPADBUTTONS);
	if((uLastButtons & MMINPUT_LANALOGS) != MMINPUT_NONE)
		m_uButtonsLatched &= (~MMINPUT_LANALOGS);
	if((uLastButtons & MMINPUT_RANALOGS) != MMINPUT_NONE)
		m_uButtonsLatched &= (~MMINPUT_RANALOGS);

	// restore the controller mapping
	gamepad_SetMapping( nControllerPort, nLastMapping );
}

// =============================================================================================================

void CMenuMgr::ReverseScroll()
{
	FASSERT(m_bIsInitialized);

	FASSERT(m_eState == MMSTATE_SCROLLING);
	// Swap the starting and ending points, and the timer.
	CFVec3 vecTemp;
	m_vecEndScrollPos *= -1.0f;
	m_fScrollTimer =  CMenuMgr_fMenuScrollTime - m_fScrollTimer;

	CMenuScreen *pTempMS = m_pNextMS;
	pTempMS = m_pNextMS;
	m_pNextMS = m_pCurMS;
	m_pCurMS = pTempMS;

	vecTemp = m_vecCursorStartPos;
	m_vecCursorStartPos = m_vecCursorEndPos;
	m_vecCursorEndPos = vecTemp;

	vecTemp = m_vecCursorStartRect;
	m_vecCursorStartRect = m_vecCursorEndRect;
	m_vecCursorEndRect = vecTemp;
}

// =============================================================================================================

void CMenuMgr::StartScroll(f32 fDir)
{
	FASSERT(m_bIsInitialized);

	u32 uDir = (fDir == -1.0f) ? 0 : 1;
	CMenuScreen *pMSNext = m_pCurMS->m_apNext[uDir];
	if(pMSNext != NULL)
	{
		FASSERT(m_eState == MMSTATE_STATIC);
		m_vecStartScrollPos = m_vecCurScrollPos;
		m_vecEndScrollPos.Set(-2.0f * fDir, 0.0f, 0.0f);
		m_fScrollTimer = 0.0f;

		m_pNextMS = pMSNext;

		m_vecCursorStartPos = m_vecCursorCurPos;
		m_vecCursorEndPos = pMSNext->m_pCurMI->m_vecBorderUL;
		m_vecCursorStartRect = m_vecCursorCurRect;
		m_vecCursorEndRect = pMSNext->m_pCurMI->m_vecBorderRect;

		m_eState = MMSTATE_SCROLLING;
	}
}

// =============================================================================================================
