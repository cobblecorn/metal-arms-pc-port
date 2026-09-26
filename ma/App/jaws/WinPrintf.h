// WinPrintf.h : header file
//

#ifndef _WINPRINTF_H_
#define _WINPRINTF_H_ 1

#include "Afxmt.h"
#include "fang.h"


#define WINPRINTF_DEFAULT_MAX_LINES 225
#define WINPRINTF_DEFAULT_MAX_LINE_LENGTH 1024
#define WINPRINTF_DEFAULT_MAX_BUFFER_LINES 225
#define WINPRINTF_STRINGLIST_BLOCKSIZE 100

#define MM_PRINTF	(WM_USER + 200)


/////////////////////////////////////////////////////////////////////////////
// CWinPrintf window

class CWinPrintf : public CEdit
{
public:
	CWinPrintf();
	void AttachToEditControl( s32 nEditControlID, CWnd *pEditControlsParentDialog, cchar *pszFontName );
	void SetParameters( s32 nMaxLines=-1, s32 nMaxLineLength=-1, s32 nMaxBufLines=-1, BOOL bShowLineNums=-1 );
	inline void Enable( BOOL bEnable ) { m_bEnable = bEnable; }
	inline BOOL IsEnabled( void ) const { return m_bEnable; }
	BOOL __cdecl Printf( const char *pszFormat, ... );
	BOOL Printf( const char *pszFormat, FANG_VA_LIST args );
	BOOL __cdecl ForcePrintf( const char *pszFormat, ... );
	BOOL ForcePrintf( const char *pszFormat, FANG_VA_LIST args );
	void Reset( void );

protected:
	BOOL _Printf( const char *pszFormat, FANG_VA_LIST args );
	void _PrintString( const char *s );

protected:
	BOOL m_bEnable;
	BOOL m_bShowLineNums;
	BOOL m_bPrintLineNum;
	int m_nLineCount;
	int m_nMaxLineLength;
	int m_nMaxLines;
	int m_nMaxBufferLines;
	CCriticalSection m_CriticalSection;
	CStringList *m_pStringList;
	CFont m_Font;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinPrintf)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWinPrintf();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWinPrintf)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg long OnPrintf( WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
