// ListCtrlWithCustomDraw.cpp : implementation file
//

#include "stdafx.h"
//#include "TestListCtrl1.h"
#include "ListCtrlWithCustomDraw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListCtrlWithCustomDraw

CListCtrlWithCustomDraw::CListCtrlWithCustomDraw()
{
}

CListCtrlWithCustomDraw::~CListCtrlWithCustomDraw()
{
}


BEGIN_MESSAGE_MAP(CListCtrlWithCustomDraw, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlWithCustomDraw)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlWithCustomDraw message handlers

void CListCtrlWithCustomDraw::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// first, lets extract data from
	// the message for ease of use later
	NMLVCUSTOMDRAW* pNMLVCUSTOMDRAW = (NMLVCUSTOMDRAW*)pNMHDR;

	// we'll copy the device context into hdc 
	// but won't convert it to a pDC* until (and if)
	// we need it as this requires a bit of work
	// internally for MFC to create temporary CDC
	// objects
	HDC hdc = pNMLVCUSTOMDRAW->nmcd.hdc;
	CDC* pDC = NULL;

	// here is the item info
	// note that we don't get the subitem
	// number here, as this may not be
	// valid data except when we are
	// handling a sub item notification
	// so we'll do that separately in
	// the appropriate case statements
	// below.
	int nItem = pNMLVCUSTOMDRAW->nmcd.dwItemSpec;
	UINT nState = pNMLVCUSTOMDRAW->nmcd.uItemState;
	LPARAM lParam = pNMLVCUSTOMDRAW->nmcd.lItemlParam;

	// next we set up flags that will control
	// the return value for *pResult
	bool bNotifyPostPaint = false;
	bool bNotifyItemDraw = false;
	bool bNotifySubItemDraw = false;
	bool bSkipDefault = false;
	bool bNewFont = false;

	// what we do next depends on the
	// drawing stage we are processing
	switch (pNMLVCUSTOMDRAW->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		{
			// PrePaint
			m_pOldItemFont = NULL;
			m_pOldSubItemFont = NULL;

			bNotifyPostPaint = IsNotifyPostPaint();
			bNotifyItemDraw = IsNotifyItemDraw();

			// do we want to draw the control ourselves?
			if (IsDraw())
			{
				if (! pDC) pDC = CDC::FromHandle(hdc);
				CRect r(pNMLVCUSTOMDRAW->nmcd.rc);

				// do the drawing
				if (OnDraw(pDC,r))
				{
					// we drew it all ourselves
					// so don't do default
					bSkipDefault = true;
				}
			}
		}
		break;

		case CDDS_ITEMPREPAINT:
		{
			// Item PrePaint
			m_pOldItemFont = NULL;

			bNotifyPostPaint = IsNotifyItemPostPaint(nItem,nState,lParam);
			bNotifySubItemDraw = IsNotifySubItemDraw(nItem,nState,lParam);

			// set up the colors to use
			pNMLVCUSTOMDRAW->clrText = TextColorForItem(nItem,nState,lParam);

			pNMLVCUSTOMDRAW->clrTextBk = BkColorForItem(nItem,nState,lParam);

			// set up a different font to use, if any
			CFont* pNewFont = FontForItem(nItem,nState,lParam);
			if(pNewFont)
			{
				if(!pDC)
					pDC = CDC::FromHandle(hdc);
				m_pOldItemFont = pDC->SelectObject(pNewFont);

				bNotifyPostPaint = true; // need to restore font
			}

			// do we want to draw the item ourselves?
			if(IsItemDraw(nItem,nState,lParam))
			{
				if(!pDC)
					pDC = CDC::FromHandle(hdc);

				if(OnItemDraw(pDC,nItem,nState,lParam))
				{
					// we drew it all ourselves
					// so don't do default
					bSkipDefault = true;
				}
			}
		}
		break;

		case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
		{
			// Sub Item PrePaint
			// set sub item number (data will be valid now)
			int nSubItem = pNMLVCUSTOMDRAW->iSubItem;

			m_pOldSubItemFont = NULL;

			bNotifyPostPaint = IsNotifySubItemPostPaint(nItem, nSubItem, nState, lParam);

			// set up the colors to use
			pNMLVCUSTOMDRAW->clrText = TextColorForSubItem(nItem,nSubItem,nState,lParam);

			pNMLVCUSTOMDRAW->clrTextBk = BkColorForSubItem(nItem,nSubItem,nState,lParam);

			// set up a different font to use, if any
			CFont* pNewFont = FontForSubItem(nItem, nSubItem, nState, lParam);

			if(pNewFont)
			{
				if(!pDC)
					pDC = CDC::FromHandle(hdc);
				m_pOldSubItemFont = pDC->SelectObject(pNewFont);

				bNotifyPostPaint = true;    // need to restore font
			}

			// do we want to draw the item ourselves?
			if (IsSubItemDraw(nItem,nSubItem,nState,lParam))
			{
				if (! pDC) pDC = CDC::FromHandle(hdc);
				if (OnSubItemDraw(pDC,nItem,nSubItem,nState,lParam))
				{
					// we drew it all ourselves
					// so don't do default
					bSkipDefault = true;
				}
			}
		}
		break;

		case CDDS_ITEMPOSTPAINT|CDDS_SUBITEM:
		{
			// Sub Item PostPaint
			// set sub item number (data will be valid now)
			int nSubItem = pNMLVCUSTOMDRAW->iSubItem;

			// restore old font if any
			if (m_pOldSubItemFont)
			{
				if(!pDC)
					pDC = CDC::FromHandle(hdc);
				pDC->SelectObject(m_pOldSubItemFont);
  
				m_pOldSubItemFont = NULL;
			}

			// do we want to do any extra drawing?
			if (IsSubItemPostDraw())
			{
				if (! pDC)
					pDC = CDC::FromHandle(hdc);
				OnSubItemPostDraw(pDC,nItem,nSubItem,nState,lParam);
			}
		}
		break;

		case CDDS_ITEMPOSTPAINT:
		{
			// Item PostPaint
			// restore old font if any
			if (m_pOldItemFont)
			{
				if (! pDC)
					pDC = CDC::FromHandle(hdc);
				pDC->SelectObject(m_pOldItemFont);
				m_pOldItemFont = NULL;
			}

			// do we want to do any extra drawing?
			if (IsItemPostDraw())
			{
				if(!pDC)
					pDC = CDC::FromHandle(hdc);
				OnItemPostDraw(pDC,nItem,nState,lParam);
			}
		}
		break;

		case CDDS_POSTPAINT:
		{
			// Item PostPaint
			// do we want to do any extra drawing?
			if(IsPostDraw())
			{
				if(!pDC)
					pDC = CDC::FromHandle(hdc);
				CRect r(pNMLVCUSTOMDRAW->nmcd.rc);

				OnPostDraw(pDC,r);
			}
		}
		break;
	}

	ASSERT(CDRF_DODEFAULT==0);
	*pResult = 0;
	if(bNotifyPostPaint)
		*pResult |= CDRF_NOTIFYPOSTPAINT;

	if(bNotifyItemDraw)
		*pResult |= CDRF_NOTIFYITEMDRAW;

	if(bNotifySubItemDraw)
		*pResult |= CDRF_NOTIFYSUBITEMDRAW;

	if(bNewFont)
		*pResult |= CDRF_NEWFONT;

	if(bSkipDefault)
		*pResult |= CDRF_SKIPDEFAULT;

	if(*pResult == 0)
	{
		// redundant as CDRF_DODEFAULT==0 anyway
		// but shouldn't depend on this in our code
		*pResult = CDRF_DODEFAULT;
	}
}
