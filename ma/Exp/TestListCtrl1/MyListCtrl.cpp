// MyListCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "TestListCtrl1.h"
#include "resource.h"
#include "MyListCtrl.h"
#include "GestureDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const u32 uGestureHeaderWidth = 100;
static const u32 uNumRows = 6;

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl

//======================================================================================

CMyListCtrl::CMyListCtrl()
{
	m_paoGestList = NULL;
	m_astrGestNames.SetSize(64);
	m_astrGestNames.RemoveAll();
	m_bInitialized = FALSE;
//	m_poSelGesture = NULL;
	m_uTopGestIdx = 0;
	m_fPlayFromPos = 2.0f;
	m_bExtDrag = FALSE;
	m_lpGI.RemoveAll();
	m_bIsDragging = FALSE;
	m_poDI = NULL;
}

//======================================================================================

CMyListCtrl::~CMyListCtrl()
{
}

//======================================================================================

void CMyListCtrl::SetPtrPos(f32 fPtrPos)
{
	m_fCursorPos = fPtrPos;
	CRect oRect;
	GetClientRect(&oRect);
	InvalidateRect(&oRect, FALSE);
}

//======================================================================================

void CMyListCtrl::SetDialogueInst(CDialogueInstN *poDI)
{
	m_poDI = poDI;
	m_paoGestList = poDI->m_aoGestureInst;
	m_astrGestNames.RemoveAll();
	m_uTopGestIdx = 0;

	// This *should* get moved somewhere else.
	if(!m_bInitialized)
	{
		CRect oRect;
		GetClientRect(&oRect);
		InsertColumn(0, "Gesture", LVCFMT_LEFT, 100, 0);
		InsertColumn(1, "", LVCFMT_LEFT, oRect.Width() - 100, 1);

		m_bInitialized = TRUE;
	}

	u32 uGIdx, uAIdx, uItemIdx;
	DeleteAllItems();
	uItemIdx = 0;
	for(uGIdx = 0; uGIdx < m_poDI->m_uNumGestures; ++uGIdx)
	{
		for(uAIdx = 0; uAIdx < uItemIdx; ++uAIdx)
		{
			if(m_astrGestNames.GetAt(uAIdx) == m_paoGestList[uGIdx].m_pGesture->m_strName)
				break;
		}
		if(uAIdx == uItemIdx)
		{
			m_astrGestNames.InsertAt(uItemIdx, m_paoGestList[uGIdx].m_pGesture->m_strName);
			InsertItem(uItemIdx, m_paoGestList[uGIdx].m_pGesture->m_strName);
			++uItemIdx;
		}
	}
}

//======================================================================================

void CMyListCtrl::SetBankPos(f32 fBankStart)
{
	m_fBankStart = fBankStart;
	CRect oRect;
	GetClientRect(&oRect);
	InvalidateRect(&oRect, FALSE);
}

//======================================================================================

void CMyListCtrl::SetBankWidth(f32 fBankWidth)
{
	m_fBankWidth = fBankWidth;
	CRect oRect;
	GetClientRect(&oRect);
	InvalidateRect(&oRect, FALSE);
}

//======================================================================================

void CMyListCtrl::SetPlayFromPos(f32 fPtrPos)
{
	m_fPlayFromPos = fPtrPos;
	CRect oRect;
	GetClientRect(&oRect);
	InvalidateRect(&oRect, FALSE);
}

//======================================================================================

void CMyListCtrl::SetExtendedDrag(BOOL bExtDrag)
{
	m_bExtDrag = bExtDrag;
	if(bExtDrag)
	{
		m_lpGI.RemoveAll();
		CRect oRect;
		GetClientRect(oRect);
		InvalidateRect(&oRect, FALSE);
	}
}

//======================================================================================

void CMyListCtrl::UpdateNames()
{
	//// Refresh the items in the control.
	//
	DeleteAllItems();
	u32 uIdx, uMax;
	uMax = FMATH_MIN(uNumRows, m_astrGestNames.GetSize());
	for(uIdx = 0; uIdx < uMax; ++uIdx)
		InsertItem(uIdx, m_astrGestNames.GetAt(uIdx + m_uTopGestIdx));
	//
	////
}

//======================================================================================

bool CMyListCtrl::IsDraw()
{
	return(true);
}

//======================================================================================

bool CMyListCtrl::OnDraw(CDC *pDC, const CRect &r)
{
	CRect oRect;
	GetClientRect(&oRect);

	CBrush brush(RGB(15, 15, 15));
	pDC->FillRect(r, &brush);

	return(false);
}

//======================================================================================

bool CMyListCtrl::IsNotifyItemDraw()
{
	return(true);
}

//======================================================================================

bool CMyListCtrl::IsNotifySubItemDraw(int nItem, UINT nState, LPARAM lParam)
{
	return(true);
}

//======================================================================================

COLORREF CMyListCtrl::TextColorForSubItem(int nItem, int nSubItem, UINT nState, LPARAM lParam)
{
	if(nSubItem)
	{
		return(RGB(15, 15, 15));
	}
	else
	{
		return(CLR_DEFAULT);
	}
}

//======================================================================================

COLORREF CMyListCtrl::BkColorForSubItem(int nItem, int nSubItem, UINT nState, LPARAM lParam)
{
	if(nSubItem == 0)
	{
		return(RGB(127, 127, 127));
	}
	else
	{
		if(((nItem + m_uTopGestIdx) & 1) ==  0)
		{
			return(RGB(63, 63, 63));
		}
		else
		{
			return(RGB(0, 0, 0));
		}
	}
	return(RGB(0, 255, 0));
}

//======================================================================================

bool CMyListCtrl::IsPostDraw()
{
	return(true);
}

//======================================================================================

bool CMyListCtrl::OnPostDraw(CDC *pDC, const CRect &r)
{
	CRect oRect;
	GetClientRect(&oRect);

	if(m_poDI == NULL)
		return(false);

	//// If the dialogue is shorter than the size of the window, mark that extra area.
	//
	if(m_fBankWidth > m_poDI->m_fTotalTime)
	{
		CRect oRect2 = oRect;
		oRect2.left = oRect.left + uGestureHeaderWidth + (s32)((m_poDI->m_fTotalTime * (f32)(oRect.Width() - uGestureHeaderWidth)) / m_fBankWidth);

		CBrush oEndBrush(RGB(47, 47, 47));
		pDC->FillRect(&oRect2, &oEndBrush);
	}
	//
	////

	//// Draw the bars representing the gestures.
	//
	u32 uIdx;
	CRect oBarRect;
	CBrush oBarBrush(RGB(191, 0, 0)), oWhiteBrush(RGB(255, 63, 63));
	for(uIdx = 0; uIdx < m_poDI->m_uNumGestures; ++uIdx)
	{
		CGestureInst *poGI = &(m_paoGestList[uIdx]);
		f32 fStartTime, fEndTime;
		fStartTime = fEndTime = poGI->m_fStartTime;
		fEndTime += poGI->m_pAnimInst->GetTotalTime();
		if((fEndTime >= m_fBankStart) && (fStartTime <= (m_fBankStart + m_fBankWidth)))
		{
			u32 uIdx, uASize;
			uASize = m_astrGestNames.GetSize();
			for(uIdx = 0; uIdx < uASize; ++uIdx)
			{
				if(m_astrGestNames.GetAt(uIdx) == poGI->m_pGesture->m_strName)
					break;
			}

			FASSERT(uIdx < uASize);

			// Only draw the bars if they are within the visible area.
			if((uIdx >= m_uTopGestIdx) && (uIdx <= (m_uTopGestIdx + uNumRows)))
			{
				const u32 uHeaderHeight = 20;
				const u32 uBarHeight = 17;

				// Set up the bar coordinates.
				oBarRect.top = oRect.top + (uIdx - m_uTopGestIdx) * uBarHeight + uHeaderHeight;
				oBarRect.bottom = oBarRect.top + uBarHeight;
				if(fStartTime <= m_fBankStart)
					oBarRect.left = oRect.left + uGestureHeaderWidth;
				else
					oBarRect.left = (u32)(((fStartTime - m_fBankStart) * (f32)(oRect.Width() - uGestureHeaderWidth)) / (m_fBankWidth)) + oRect.left + uGestureHeaderWidth;
				if(fEndTime >= (m_fBankStart + m_fBankWidth))
					oBarRect.right = oRect.right;
				else
					oBarRect.right = (u32)(((fEndTime - m_fBankStart) * (f32)(oRect.Width() - uGestureHeaderWidth)) / (m_fBankWidth)) + oRect.left + uGestureHeaderWidth;

				// Draw the bar.
				pDC->FillRect(&oBarRect, &oBarBrush);
				// Draw the text on the bars.
				pDC->SetTextColor(RGB(255, 255, 255));
				pDC->DrawText(poGI->m_pGesture->m_strName, &oBarRect, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS);
				// Draw the highlight around it if is one of the selected gestures.
				if(m_lpGI.Find(poGI) != NULL)
				{
					pDC->FrameRect(&oBarRect, &oWhiteBrush);
				}
			}
		}
	}
	//
	////

	//// Draw the cursors.
	//
	// Draw the 'play from' cursor first.
	if((m_fPlayFromPos >= m_fBankStart) && (m_fPlayFromPos <= (m_fBankStart + m_fBankWidth)))
	{
		CBrush oCursBrush(RGB(255, 0, 0));
		int nXPos = (int)(((m_fPlayFromPos - m_fBankStart) * (f32)(oRect.Width() - uGestureHeaderWidth)) / (m_fBankWidth)) + oRect.left + uGestureHeaderWidth;
		CRect oRect2;
		oRect2.left = nXPos;
		oRect2.top = oRect.top;
		oRect2.right = nXPos + 1;
		oRect2.bottom = oRect.bottom;
		pDC->FillRect(oRect2, &oCursBrush);
	}
	// Then draw the regular play cursor.
	if((m_fCursorPos >= m_fBankStart) && (m_fCursorPos <= (m_fBankStart + m_fBankWidth)))
	{
		CBrush oCursBrush(RGB(255, 255, 255));
		int nXPos = (int)(((m_fCursorPos - m_fBankStart) * (f32)(oRect.Width() - uGestureHeaderWidth)) / (m_fBankWidth)) + oRect.left + uGestureHeaderWidth;
		CRect oRect2;
		oRect2.left = nXPos;
		oRect2.top = oRect.top;
		oRect2.right = nXPos + 1;
		oRect2.bottom = oRect.bottom;
		pDC->FillRect(oRect2, &oCursBrush);
	}
	//
	////

	return(false);
}

//======================================================================================

bool CMyListCtrl::IsNotifyPostPaint()
{
	return(TRUE);
}

//======================================================================================

BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrlWithCustomDraw)
	//{{AFX_MSG_MAP(CMyListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_COMMAND(ID_INSERTNEWGEST, OnInsertNewGest)
	ON_COMMAND(ID_INSERTTHISGEST, OnInsertThisGest)
	ON_COMMAND(ID_DELETEGEST, OnDeleteGest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl message handlers

//======================================================================================

void CMyListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
//	CListCtrlWithCustomDraw::OnLButtonDown(nFlags, point);
	CRect oRect;
	GetClientRect(&oRect);

	s32 nRow = (point.y - 18) / 18 + m_uTopGestIdx;
	if(nRow < m_astrGestNames.GetSize())
	{
		m_oLClicked = point;
		CString strGestName = m_astrGestNames.GetAt(nRow);
		f32 fTimePos = ((m_fBankWidth) * (f32)(point.x - uGestureHeaderWidth) / (oRect.Width() - uGestureHeaderWidth)) + m_fBankStart;

		//// See if there is a valid gesture here.
		//
		// CHANGED:
/*		u32 uIdx;
		for(uIdx = 0; uIdx < m_poDI->m_uNumGestures; ++uIdx)
		{
			if((m_paoGestList[uIdx].m_pGesture->m_strName == strGestName) && (m_paoGestList[uIdx].m_fStartTime < fTimePos) && (m_paoGestList[uIdx].m_fStartTime + m_paoGestList[uIdx].m_pAnimInst->GetTotalTime() > fTimePos))
				break;
		}*/
		//
		////

		//// If we found a valid gesture, mark all appropriate gestures.
		//
		CGestureInst *pHitGI;		// This is the GI that they clicked on.
		if(m_poDI->FindGestureAtTime(fTimePos, strGestName, &pHitGI))
		{
//		if(uIdx != m_poDI->m_uNumGestures)
//		{
			if(m_bExtDrag)
			{
//				f32 fTargetTime = m_paoGestList[uIdx].m_fStartTime;
				f32 fTargetTime = pHitGI->m_fStartTime;
				u32 uGIdx;
				for(uGIdx = 0; uGIdx < m_poDI->m_uNumGestures; ++uGIdx)
				{
					if(m_paoGestList[uGIdx].m_fStartTime >= fTargetTime)
					{
						m_lpGI.AddTail(&(m_paoGestList[uGIdx]));
					}
				}
			}
			else
			{
				POSITION hPos = m_lpGI.Find(pHitGI);
				if(hPos == NULL)
				{
					m_lpGI.AddTail(pHitGI);
				}
				else
				{
					m_lpGI.RemoveAt(hPos);
				}
			}

			InvalidateRect(&oRect, FALSE);
			m_bIsDragging = TRUE;
			SetCapture();
		}
		//
		////
	}
}

//======================================================================================

void CMyListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bIsDragging)
	{
		if((m_bExtDrag) || !(nFlags & MK_CONTROL))
		{
			m_lpGI.RemoveAll();
		}
		ReleaseCapture();
		m_bIsDragging = FALSE;
		CRect oRect;
		GetClientRect(&oRect);
		InvalidateRect(&oRect, FALSE);
	}
}

//======================================================================================

void CMyListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bIsDragging)
	{
		CRect oRect;
		GetClientRect(&oRect);

		if(point.x > oRect.right)
			point.x = oRect.right;
		if(point.x < (s32)(oRect.left + uGestureHeaderWidth))
			point.x = oRect.left + uGestureHeaderWidth;

		// Get the current time pos, and don't allow them to be out of the current bank.
		f32 fTimePos = ((m_fBankWidth) * (f32)(point.x - uGestureHeaderWidth) / (oRect.Width() - uGestureHeaderWidth)) + m_fBankStart;
		if(fTimePos > (m_fBankStart + m_fBankWidth))
			FASSERT_NOW;		

		f32 fOldTimePos = ((m_fBankWidth) * (f32)(m_oLClicked.x - uGestureHeaderWidth) / (oRect.Width() - uGestureHeaderWidth)) + m_fBankStart;
		f32 fDelTimePos = fTimePos - fOldTimePos;
		
		//////////////////////////////////////////////////////////////////////
		// Go through each of the gestures in the selected list and adjust
		//   them based on the mouse movement.
		POSITION hPos = m_lpGI.GetHeadPosition();
		CGestureInst *poGI;
		while(hPos != NULL)
		{
			poGI = (CGestureInst *)(m_lpGI.GetAt(hPos));
			CGestureInst *poTempGI;
			//////////////////////////////////////////////////////////////////////
			// Ensure that two gestures on the same line don't collide.
			if(fDelTimePos > 0.0f)
			{
				// Check for collision on the right side.
				if(m_poDI->FindGestureAtTime(poGI->m_fStartTime + fDelTimePos + poGI->m_pAnimInst->GetTotalTime(), poGI->m_pGesture->m_strName, &poTempGI))
				{
					// Adjust the position of the gesture instance.
					poGI->m_fStartTime = poTempGI->m_fStartTime - poGI->m_pAnimInst->GetTotalTime();
				}
				else
				{
					// The coast was clear, we'll go ahead and move the gesture.
					poGI->m_fStartTime += fDelTimePos;
				}

				if(poGI->m_fStartTime > m_poDI->m_fTotalTime - 0.01f)
				{
					poGI->m_fStartTime = m_poDI->m_fTotalTime - 0.01f;
				}
			}
			else if(fDelTimePos < 0.0f)
			{
				// Check for collision on the left side.
				if(m_poDI->FindGestureAtTime(poGI->m_fStartTime + fDelTimePos, poGI->m_pGesture->m_strName, &poTempGI))
				{
					// Adjust the position of the gesture instance.
					poGI->m_fStartTime = poTempGI->m_fStartTime + poTempGI->m_pAnimInst->GetTotalTime();
				}
				else
				{
					// The coast was clear, we'll go ahead and move the gesture.
					poGI->m_fStartTime += fDelTimePos;
				}
			}
			//
			//////////////////////////////////////////////////////////////////////
			m_lpGI.GetNext(hPos);
		}
		//
		//////////////////////////////////////////////////////////////////////

		m_poDI->RecalcUserAnimSlots();
		m_oLClicked = point;
		InvalidateRect(&oRect, FALSE);
	}
	CListCtrlWithCustomDraw::OnMouseMove(nFlags, point);
}

//======================================================================================

void CMyListCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Create the popup menu
	CMenu oMenu;
	oMenu.LoadMenu(IDR_MENU_RCLICK);
	CMenu *poContextMenu = oMenu.GetSubMenu(0);

	CPoint Point;
	::GetCursorPos(&Point);
	ScreenToClient(&Point);

	CRect oRect;
	GetClientRect(&oRect);

	m_oRClicked = Point;
	s32 nRow = (Point.y - 18) / 18 + m_uTopGestIdx;
	if(nRow < m_astrGestNames.GetSize())
	{
		CString strGestName = m_astrGestNames.GetAt(nRow);
		f32 fTimePos = ((m_fBankWidth) * (f32)(Point.x - uGestureHeaderWidth) / (oRect.Width() - uGestureHeaderWidth)) + m_fBankStart;

/*		u32 uIdx;
		for(uIdx = 0; uIdx < m_poDI->m_uNumGestures; ++uIdx)
		{
			if((m_paoGestList[uIdx].m_pGesture->m_strName == strGestName) && (m_paoGestList[uIdx].m_fStartTime < fTimePos) && (m_paoGestList[uIdx].m_fStartTime + m_paoGestList[uIdx].m_pGesture->m_pAnimInst->GetTotalTime() > fTimePos))
				break;
		}*/

		// Search through the GestureInst list for find if there
		CGestureInst *pHitGI;		// This is the GI that they clicked on.
		if(m_poDI->FindGestureAtTime(fTimePos, strGestName, &pHitGI))
		{
//		if(uIdx != m_poDI->m_uNumGestures)
//		{
			// We need to mark on which gesture they clicked in case we want to delete it.
			m_lpGI.RemoveAll();
//			m_lpGI.AddTail(&(m_paoGestList[uIdx]));
			m_lpGI.AddTail(pHitGI);
			poContextMenu->EnableMenuItem(ID_INSERTTHISGEST, MF_GRAYED);
			poContextMenu->EnableMenuItem(ID_DELETEGEST, MF_ENABLED);
		}
		else
		{
			poContextMenu->EnableMenuItem(ID_INSERTTHISGEST, MF_ENABLED);
			poContextMenu->EnableMenuItem(ID_DELETEGEST, MF_GRAYED);
		}
	}
	else
	{
			poContextMenu->EnableMenuItem(ID_INSERTTHISGEST, MF_GRAYED);
			poContextMenu->EnableMenuItem(ID_DELETEGEST, MF_GRAYED);
	}

	// Display the popup menu
	ClientToScreen(&Point);
	poContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this);
	
	*pResult = 0;
}

//======================================================================================

void CMyListCtrl::OnInsertNewGest()
{
	CGestureDialog oGestDialog;

	CGesture *poG;
	if(oGestDialog.DoGetNewGesture(CDialogueListN::s_pCurDE->m_poCharInfo, &poG, &m_astrGestNames))
	{
		// JUSTIN: Fix this calc to work for small fonts.
		s32 nRow = (m_oRClicked.y - 18) / 18 + m_uTopGestIdx;
		s32 nSize = m_astrGestNames.GetSize();
		if(nRow > nSize)
		{
			nRow = nSize;
			// Fix up the point that we'll say that they clicked on because we're going to insert
			//   a gesture here in a second.
			m_oRClicked.y = ((nRow - m_uTopGestIdx) * 18) + 18;
		}
		m_astrGestNames.InsertAt(nRow, poG->m_strName);
		OnInsertThisGest();
		UpdateNames();
		m_bNewScrollInfo = TRUE;
	}
}

//======================================================================================

void CMyListCtrl::OnInsertThisGest()
{
	CRect oRect;
	GetClientRect(&oRect);

	s32 nRow = (m_oRClicked.y - 18) / 18 + m_uTopGestIdx;
	FASSERT(nRow < m_astrGestNames.GetSize());
	CString strGestName = m_astrGestNames.GetAt(nRow);
	CGesture *poGesture;
	if(!m_poDI->m_pDE->m_poCharInfo->FindGesture(strGestName, &poGesture))
	{
		return;
	}
	f32 fTimePos = ((m_fBankWidth) * (f32)(m_oRClicked.x - uGestureHeaderWidth) / (oRect.Width() - uGestureHeaderWidth)) + m_fBankStart;
	if(!m_poDI->AddGesture(poGesture, fTimePos))
	{
		return;
	}

	InvalidateRect(&oRect, FALSE);
	m_bNewGestureSet = TRUE;
}

//======================================================================================

void CMyListCtrl::OnDeleteGest()
{
	FASSERT(!m_lpGI.IsEmpty());
	FASSERT(m_lpGI.GetCount() == 1);
	if(!m_poDI->RemoveGesture((CGestureInst *)(m_lpGI.GetHead())))
	{
		FASSERT_NOW;
	}
	m_lpGI.RemoveTail();
	FASSERT(m_lpGI.IsEmpty());

	CRect oRect;
	GetClientRect(&oRect);
	InvalidateRect(&oRect, FALSE);
	m_bNewGestureSet = TRUE;
	m_bNewScrollInfo = TRUE;
}

//======================================================================================

BOOL CMyListCtrl::NewGestureSet()
{
	if(m_bNewGestureSet)
	{
		m_bNewGestureSet = FALSE;
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

//======================================================================================

BOOL CMyListCtrl::NewScrollInfo()
{
	// JUSTIN : Temporary.
//	return(TRUE);
	if(m_bNewScrollInfo)
	{
		m_bNewScrollInfo = FALSE;
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

//======================================================================================

void CMyListCtrl::GetNewScrollInfo(SCROLLINFO *poSI)
{
	poSI->cbSize = sizeof(SCROLLINFO);
	poSI->fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	poSI->nMin = 0;
	poSI->nMax = m_astrGestNames.GetSize() - 1;
	poSI->nPage = uNumRows;
	poSI->nPos = m_uTopGestIdx;
}

//======================================================================================

void CMyListCtrl::OnVScrollExt(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(m_astrGestNames.GetSize() > uNumRows)
	{
		CRect oRect;
		GetClientRect(&oRect);

		switch(nSBCode)
		{
			case SB_LINEDOWN:
			{
				if(m_uTopGestIdx < (m_astrGestNames.GetSize() - uNumRows))
				{
					++m_uTopGestIdx;
					m_bNewScrollInfo = TRUE;
					InvalidateRect(&oRect, FALSE);
					UpdateNames();
				}
				break;
			}
			case SB_LINEUP:
			{
				if(m_uTopGestIdx > 0)
				{
					--m_uTopGestIdx;
					m_bNewScrollInfo = TRUE;
					InvalidateRect(&oRect, FALSE);
					UpdateNames();
				}
				break;
			}
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
			{
				m_uTopGestIdx = nPos;
				m_bNewScrollInfo = TRUE;
				InvalidateRect(&oRect, FALSE);
				UpdateNames();
				break;
			}
		}
	}
}