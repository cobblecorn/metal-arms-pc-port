#if !defined(AFX_MYLISTCTRL_H__D8A6520D_88AF_4119_9923_8D2E84BBCBCF__INCLUDED_)
#define AFX_MYLISTCTRL_H__D8A6520D_88AF_4119_9923_8D2E84BBCBCF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyListCtrl.h : header file
//

#include "ListCtrlWithCustomDraw.h"
#include "fang.h"
#include "dialogue.h"

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl window

class CMyListCtrl : public CListCtrlWithCustomDraw
{
// Construction
public:
	CMyListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyListCtrl)
	//}}AFX_VIRTUAL

	void SetPtrPos(f32 fPtrPos);
	void SetDialogueInst(CDialogueInstN *poDI);
	void SetBankPos(f32 fBankStart);
	void SetBankWidth(f32 fBankWidth);
	void SetPlayFromPos(f32 fPtrPos);
	void SetExtendedDrag(BOOL bExtDrag);
	void UpdateNames();
	void OnVScrollExt(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	BOOL NewGestureSet();
	BOOL NewScrollInfo();
	void GetNewScrollInfo(SCROLLINFO *poSI);

protected:
	virtual bool IsDraw();
	virtual bool OnDraw(CDC *pDC, const CRect &r);
	virtual bool IsNotifyItemDraw();
	virtual bool IsNotifySubItemDraw(int nItem, UINT nState, LPARAM lParam);
	virtual COLORREF CMyListCtrl::TextColorForSubItem(int nItem, int nSubItem, UINT nState, LPARAM lParam);
	virtual COLORREF CMyListCtrl::BkColorForSubItem(int nItem, int nSubItem, UINT nState, LPARAM lParam);
	virtual bool IsPostDraw();
	virtual bool OnPostDraw(CDC *pDC, const CRect &r);
	virtual bool IsNotifyPostPaint();

	CDialogueInstN *m_poDI;
	CGestureInst *m_paoGestList;
	CStringArray m_astrGestNames;
	BOOL m_bInitialized, m_bNewGestureSet, m_bNewScrollInfo, m_bExtDrag;

	f32 m_fCursorPos, m_fPlayFromPos;
	f32 m_fBankStart, m_fBankWidth;

//	CGestureInst *m_poSelGesture;
//	f32 m_fSelDeltaTime;
	CPtrList m_lpGI;					// This is a collection of GestureInstances.  We need to keep this
										//   because the user has the option of selecting multiple gestures
										//   at once, and then performing an operation on them (such as move
										//   or delete).
	BOOL m_bIsDragging;
	u32 m_uTopGestIdx;
	CPoint m_oRClicked;					// Set in the right click handler, used by the menu option handlers.
	CPoint m_oLClicked;					// Set in the left button down handler, used by the mouse move handler.

// Implementation
public:
	virtual ~CMyListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyListCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnInsertNewGest();
	afx_msg void OnInsertThisGest();
	afx_msg void OnDeleteGest();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYLISTCTRL_H__D8A6520D_88AF_4119_9923_8D2E84BBCBCF__INCLUDED_)
