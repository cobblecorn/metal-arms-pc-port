// WinPrintf.cpp : implementation file
//

#include "stdafx.h"
#include "Afxmt.h"
#include "WinPrintf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinPrintf

CWinPrintf::CWinPrintf() {
	m_nMaxLines = WINPRINTF_DEFAULT_MAX_LINES;
	m_nMaxLineLength = WINPRINTF_DEFAULT_MAX_LINE_LENGTH;
	m_bShowLineNums = TRUE;
	m_nLineCount = 1;
	m_bPrintLineNum = TRUE;
	m_nMaxBufferLines = WINPRINTF_DEFAULT_MAX_BUFFER_LINES;

	m_pStringList = new CStringList( WINPRINTF_STRINGLIST_BLOCKSIZE );
}

CWinPrintf::~CWinPrintf() {
	delete m_pStringList;	// deletes all CString objects as well.
}

void CWinPrintf::AttachToEditControl( s32 nEditControlID, CWnd *pEditControlsParentDialog, cchar *pszFontName ) {
	SubclassDlgItem( nEditControlID, pEditControlsParentDialog );

	if( pszFontName ) {
		if( pszFontName[0] == 0 ) {
			// Default to courier new...
			pszFontName = "Courier New";
		}

		m_Font.CreateFont( 14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, pszFontName );
		pEditControlsParentDialog->GetDlgItem( nEditControlID )->SetFont( &m_Font, FALSE );
	}
}

// Set any value to -1 if the current value is not to be changed.
void CWinPrintf::SetParameters( s32 nMaxLines, s32 nMaxLineLength, s32 nMaxBufLines, BOOL bShowLineNums ) {
	ASSERT( nMaxLines>=-1 );
	ASSERT( nMaxLineLength>=-1 );

	m_CriticalSection.Lock();

	if( nMaxLines != -1 ) m_nMaxLines = nMaxLines;
	if( nMaxLineLength != -1 ) m_nMaxLineLength = nMaxLineLength;
	if( nMaxBufLines != -1 ) m_nMaxBufferLines = nMaxBufLines;
	if( bShowLineNums != -1 ) m_bShowLineNums = bShowLineNums;

	int nLastLine = GetLineCount() - 1;

	while( nLastLine > m_nMaxLines ) {
		int first_line_len = LineLength( 0 );
		SetSel( 0, LineLength(0)+2, TRUE );
		ReplaceSel( "" );
		nLastLine--;
	}

	m_CriticalSection.Unlock();
}


// Returns TRUE if successful.
// Returns FALSE if print buffer is full (call again after a Sleep(0), for example)
BOOL __cdecl CWinPrintf::Printf( const char *pszFormat, ... ) {
	BOOL r = TRUE;

	if( m_bEnable ) {
		FANG_VA_LIST args;
		FANG_VA_START( args, pszFormat );
		r = _Printf( pszFormat, args );
		va_end(args);
	}

	return r;
}

// Returns TRUE if successful.
// Returns FALSE if print buffer is full (call again after a Sleep(0), for example)
BOOL CWinPrintf::Printf( const char *pszFormat, FANG_VA_LIST args ) {
	BOOL r = TRUE;

	if( m_bEnable ) {
		r = _Printf( pszFormat, args );
	}

	return r;
}

// Returns TRUE if successful.
// Returns FALSE if print buffer is full (call again after a Sleep(0), for example)
BOOL __cdecl CWinPrintf::ForcePrintf( const char *pszFormat, ... ) {
	BOOL r;

	FANG_VA_LIST args;
	FANG_VA_START( args, pszFormat );
	r = _Printf( pszFormat, args );
	va_end(args);

	return r;
}

// Returns TRUE if successful.
// Returns FALSE if print buffer is full (call again after a Sleep(0), for example)
BOOL CWinPrintf::ForcePrintf( const char *pszFormat, FANG_VA_LIST args ) {
	return _Printf( pszFormat, args );
}

// Returns TRUE if successful.
// Returns FALSE if print buffer is full (call again after a Sleep(0), for example)
BOOL CWinPrintf::_Printf( const char *pszFormat, FANG_VA_LIST args ) {
	CString s;

	m_CriticalSection.Lock();
	if( m_pStringList->GetCount() >= m_nMaxBufferLines ) return FALSE;

	try {
		_vstprintf( s.GetBuffer( m_nMaxLineLength+1 ), pszFormat, args );
		s.ReleaseBuffer();
		m_pStringList->AddTail( s );
	}
	catch( CMemoryException *e ) {
		m_CriticalSection.Unlock();
		e->Delete();
		return FALSE;
	}

	m_CriticalSection.Unlock();
	PostMessage( MM_PRINTF, 0, 0 );
	return TRUE;
}

void CWinPrintf::_PrintString( const char *s ) {
	SetRedraw( FALSE );

	int nLastLine = GetLineCount() - 1;

	if( nLastLine >= m_nMaxLines ) {
		int first_line_len = LineLength( 0 );
		SetSel( 0, LineLength(0)+2, TRUE );
		ReplaceSel( "" );
		nLastLine--;
	}

	SetRedraw( TRUE );
	Invalidate( FALSE );

	if( nLastLine >= 0 ) {
		int last_char_index = LineIndex( nLastLine ) + LineLength( nLastLine );
		SetSel( last_char_index, last_char_index, FALSE );
	} else {
		SetSel( 0, 0, FALSE );
	}

	char *c, n[50];
	while( c = strchr( s, '\x00a' ) ) {
		if( m_bShowLineNums && m_bPrintLineNum ) {
			sprintf( n, "%4i ", m_nLineCount );
			ReplaceSel( n );
		}

		*c = 0;
		ReplaceSel( s );
		*c = '\x00a';
		ReplaceSel( "\x00d\x00a" );
		s = c+1;
		m_nLineCount++;
		m_bPrintLineNum = TRUE;
	}

	if( c != s ) {
		if( m_bPrintLineNum ) {
			if( m_bShowLineNums ) {
				sprintf( n, "%4i ", m_nLineCount );
				ReplaceSel( n );
			}
			m_bPrintLineNum = FALSE;
		}
		ReplaceSel( s );
	}
}

void CWinPrintf::Reset( void ) {
	m_CriticalSection.Lock();
	SetWindowText( "" );
	m_nLineCount = 1;
	m_bPrintLineNum = TRUE;
	m_CriticalSection.Unlock();
}

BEGIN_MESSAGE_MAP(CWinPrintf, CEdit)
	//{{AFX_MSG_MAP(CWinPrintf)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_MESSAGE(MM_PRINTF, OnPrintf)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinPrintf message handlers


void CWinPrintf::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if( nChar == VK_ESCAPE ) return;

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CWinPrintf::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if( nChar == VK_ESCAPE ) return;

	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

long CWinPrintf::OnPrintf( WPARAM wParam, LPARAM lParam ) {
	CString s;

	m_CriticalSection.Lock();

	while( !m_pStringList->IsEmpty() ) {
		s = m_pStringList->RemoveHead();
		_PrintString( s );
	}

	m_CriticalSection.Unlock();
	return 0;
}

