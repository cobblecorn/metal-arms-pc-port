// GestureDialog.cpp : implementation file
//

#include "stdafx.h"
#include "bottalkinator.h"
#include "GestureDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGestureDialog dialog


CGestureDialog::CGestureDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGestureDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGestureDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGestureDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGestureDialog)
	DDX_Control(pDX, IDC_COMBO_GESTID, m_oComboGestID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGestureDialog, CDialog)
	//{{AFX_MSG_MAP(CGestureDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGestureDialog message handlers

// DoGetNewGesture should get called instead of calling DoModal().  The reason for this is that you
//   need to pass to the Dialog some information with which it can generate the gestures for the combo box.
BOOL CGestureDialog::DoGetNewGesture(CCharacterInfo *poCI, CGesture **ppoG, CStringArray *pastrGestNames)
{
	m_poCI = poCI;
	m_poG = NULL;
	m_pastrGestNames = pastrGestNames;

	// Here we just check to see if there are any gestures available to be added. If there aren't, we bail out below.
	u32 uCurIdx, uSearchIdx;
	BOOL bFoundGesture = FALSE;
	for(uCurIdx = 0; uCurIdx < m_poCI->m_uNumGestures; ++uCurIdx)
	{
		u32 uNumGests = m_pastrGestNames->GetSize();
		for(uSearchIdx = 0; uSearchIdx < uNumGests; ++uSearchIdx)
		{
			if(m_poCI->m_aoGestures[uCurIdx].m_strName.CompareNoCase( m_pastrGestNames->GetAt(uSearchIdx) ) == 0)
			{
				break;
			}
		}
		if(uSearchIdx == uNumGests)
		{
			bFoundGesture = TRUE;
			break;
		}
	}
	
	// Are there any gestures in the list that weren't already in use?
	int nResponse;
	if( !bFoundGesture ) {
		// Nope, let's let them know and bail out.
		AfxMessageBox("There are no more unlisted gestures to add.");
		return(FALSE);
	} else {
		// We found at least one, let's do the usual business.
		nResponse = DoModal();
	}
	
	// Let the caller know what happened.
	*ppoG = m_poG;
	if( nResponse == IDOK ) {
		return(TRUE);
	} else {
		return(FALSE);
	}

	return(TRUE);	
}

BOOL CGestureDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_oComboGestID.ResetContent();

	// Populate our combo box with valid gestures.
	u32 uCurIdx, uSearchIdx;
	for( uCurIdx = 0; uCurIdx < m_poCI->m_uNumGestures; ++uCurIdx ) {
		u32 uNumGests = m_pastrGestNames->GetSize();
		
		// Check to see if a gesture by this name is already there or not.
		for( uSearchIdx = 0; uSearchIdx < uNumGests; ++uSearchIdx ) {
			if( m_poCI->m_aoGestures[uCurIdx].m_strName.CompareNoCase( m_pastrGestNames->GetAt(uSearchIdx) ) == 0 ) {
				break;
			}
		}

		// We didn't find it, so let's add it.
		if( uSearchIdx == uNumGests ) {
			m_oComboGestID.AddString(m_poCI->m_aoGestures[uCurIdx].m_strName);
		}		
	}
	
	m_oComboGestID.SetCurSel(0);

	UpdateData( VARS_TO_CONTROLS );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGestureDialog::OnOK() {
	CString strTemp;
	m_oComboGestID.GetWindowText( strTemp );
	m_poCI->FindGesture( strTemp, &m_poG );
	
	CDialog::OnOK();
}
