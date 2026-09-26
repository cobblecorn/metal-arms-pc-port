#if !defined(AFX_LISTCTRLWITHCUSTOMDRAW_H__AD40B994_BD18_4DCD_8B2D_9358F1E732C4__INCLUDED_)
#define AFX_LISTCTRLWITHCUSTOMDRAW_H__AD40B994_BD18_4DCD_8B2D_9358F1E732C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrlWithCustomDraw.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListCtrlWithCustomDraw window

class CListCtrlWithCustomDraw : public CListCtrl
{
// Construction
public:
	CListCtrlWithCustomDraw();

// Attributes
public:

protected:
	CFont *m_pOldItemFont;
	CFont *m_pOldSubItemFont;

	// Do we want to do the drawing ourselves?
	virtual bool IsDraw() { return(false); }

	virtual bool OnDraw(CDC *pDC, const CRect &r) { return(false); }

	virtual bool IsNotifyItemDraw() { return(false); }

	virtual bool IsNotifyPostPaint() { return(false); }

	virtual bool IsPostDraw() { return(false); }

	virtual bool OnPostDraw(CDC *pDC, const CRect &r) { return(false); }

	virtual CFont *FontForItem(int nItem, UINT nState, LPARAM lParam) { return(NULL); }

	virtual COLORREF TextColorForItem(int nItem, UINT nState, LPARAM lParam) { return(CLR_DEFAULT); }

	virtual COLORREF BkColorForItem(int nItem, UINT nState, LPARAM lParam) { return(CLR_DEFAULT); }

	virtual bool IsItemDraw(int nItem, UINT nState, LPARAM lParam) { return(false); }

	virtual bool OnItemDraw(CDC *pDC, int nItem, UINT nState, LPARAM lParam) { return(false); }

	virtual bool IsNotifySubItemDraw(int nItem, UINT nState, LPARAM lParam) { return(false); }

	virtual bool IsNotifyItemPostPaint(int nItem, UINT nState, LPARAM lParam) { return(false); }

	virtual bool IsItemPostDraw() { return(false); }

	virtual bool OnItemPostDraw(CDC *pDC, int nItem, UINT nState, LPARAM lParam) { return(false); }

	virtual CFont *FontForSubItem(int nItem, int nSubItem, UINT nState, LPARAM lParam) { return(NULL); }

	virtual COLORREF TextColorForSubItem(int nItem, int nSubItem, UINT nState, LPARAM lParam) { return(CLR_DEFAULT); }

	virtual COLORREF BkColorForSubItem(int nItem, int nSubItem, UINT nState, LPARAM lParam) { return(CLR_DEFAULT); }

	virtual bool IsSubItemDraw(int nItem, int nSubItem, UINT nState, LPARAM lParam) { return(false); }

	virtual bool OnSubItemDraw(CDC *pDC, int nItem, int nSubItem, UINT nState, LPARAM lParam) { return(false); }

	virtual bool IsNotifySubItemPostPaint(int nItem, int nSubItem, UINT nState, LPARAM lParam) { return(false); }

	virtual bool IsSubItemPostDraw() { return(false); }

	virtual bool OnSubItemPostDraw(CDC *pDC, int nItem, int nSubItem, UINT nState, LPARAM lParam) { return(false); }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlWithCustomDraw)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListCtrlWithCustomDraw();

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrlWithCustomDraw)
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRLWITHCUSTOMDRAW_H__AD40B994_BD18_4DCD_8B2D_9358F1E732C4__INCLUDED_)
