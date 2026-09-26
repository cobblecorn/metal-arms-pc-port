//////////////////////////////////////////////////////////////////////////////////////
// InputEmulation.h - InputEmulation implementation.
//
// Author: Albert Yale
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/23/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// InputEmulation.cpp : implementation file
//

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "stdafx.h"
#include "ma_win.h"
#include "InputEmulation.h"
#include "settings.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/////////////////////////////////////////////////////////////////////////////
// CInputEmulation dialog

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CInputEmulation::CInputEmulation(CWnd* pParent /*=NULL*/)
	: CDialog(CInputEmulation::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInputEmulation)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CInputEmulation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInputEmulation)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BEGIN_MESSAGE_MAP(CInputEmulation, CDialog)
	//{{AFX_MSG_MAP(CInputEmulation)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/////////////////////////////////////////////////////////////////////////////
// CInputEmulation message handlers

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _TIMER_CONTROL_SAMPLING		( WM_USER + 0 )
#define _TIMER_DELAY				17

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int CInputEmulation::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( CDialog::OnCreate( lpCreateStruct ) == -1 ) return -1;

	FPadio_Init_t oPadioInit;
	fang_MemZero( &oPadioInit, sizeof( oPadioInit ) );
	oPadioInit.DX8ONLY_ohWnd = (u32)GetSafeHwnd();
	oPadioInit.DX8ONLY_ohInstance = (u32)AfxGetInstanceHandle();
	oPadioInit.DX8ONLY_pauInputEmulationMap = NULL;
	oPadioInit.DX8ONLY_uInputEmulationPlatform = FPADIO_INPUT_EMULATION_PLATFORM_NONE;

	if( FPADIO_NO_ERROR != fpadio_Install( &oPadioInit ) )
	{
		MessageBox( "Could not install input driver.", "ERROR", MB_ICONSTOP );
		return -1;
	}

	return 0;

} // CInputEmulation::OnCreate

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CInputEmulation::OnDestroy() 
{
	CDialog::OnDestroy();

	if( fpadio_IsInstalled() )
	{
		KillTimer( _TIMER_CONTROL_SAMPLING );

		fpadio_Uninstall();
	}

} // CInputEmulation::OnDestroy

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CInputEmulation::OnInitDialog()
{
	CDialog::OnInitDialog();

	fpadio_GetSamples( &m_uValidSamples, &m_paoSamples );

	if( ! m_uValidSamples )
	{
		Sleep( _TIMER_DELAY );
		fpadio_GetSamples( &m_uValidSamples, &m_paoSamples );
		if( m_uValidSamples )
		{
			m_uSampledItems = 0;

			for( m_uDev = 0; m_uDev < FPADIO_MAX_DEVICES; ++m_uDev )
			{
				if( m_paoSamples[ m_uDev ][ m_uValidSamples - 1 ].bValid )
				{
					++m_uSampledItems;
				}
			}

			if( ! m_uSampledItems )
			{
				MessageBox( "No devices connected\n\nClose and open the Input Setup again after connecting a controller.", "Error", MB_ICONSTOP );
			}
		}
	}

	m_oPreviousSelection = "";

	m_uDelay = 0;

	SetTimer( _TIMER_CONTROL_SAMPLING, _TIMER_DELAY, NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

} // CInputEmulation::OnInitDialog

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CInputEmulation::OnTimer( UINT nIDEvent )
{
	fpadio_GetSamples( &m_uValidSamples, &m_paoSamples );

	if( ! m_uValidSamples ) return;

if( ((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->GetDroppedState() )
return;

	////
	//
	if( 2 > m_uDelay )
	{
		m_bResetContent = FALSE;
		m_uComboboxItems = ((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->GetCount();
/*
		if( CB_ERR == m_uComboboxItems )
		{
			return;
		}
*/
		m_uSampledItems = 0;

		for( m_uDev = 0; m_uDev < FPADIO_MAX_DEVICES; ++m_uDev )
		{
			if( m_paoSamples[ m_uDev ][ m_uValidSamples - 1 ].bValid )
			{
				++m_uSampledItems;

				fpadio_GetDeviceInfo( m_uDev, &m_oDeviceInfo );
				m_oString = m_oDeviceInfo.szName;
				if( CB_ERR == ((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->FindString( -1, m_oString ) )
				{
					m_bResetContent = TRUE;
					break;
				}
			}
		}

		if( m_uComboboxItems != m_uSampledItems )
		{
			m_bResetContent = TRUE;
		}

		if( m_bResetContent )
		{
			((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->ResetContent();

			for( m_uDev = 0; m_uDev < FPADIO_MAX_DEVICES; ++m_uDev )
			{
				if( m_paoSamples[ m_uDev ][ m_uValidSamples - 1 ].bValid )
				{
					fpadio_GetDeviceInfo( m_uDev, &m_oDeviceInfo );
					m_oString = m_oDeviceInfo.szName;
					((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->AddString( m_oString );
				}
			}

			if( m_uSampledItems )
			{
				if( 0 == m_uDelay ) // Inital setup.
				{
					CSettings& oSettings = CSettings::GetCurrent();

					u32 uIndex = ((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->FindString( -1, oSettings.m_sInputEmulationDevName );
					if( CB_ERR == uIndex )
					{
						++m_uDelay;
						SetComboboxSelection( IDC_COMBO_EMUL_DEVICES, 0 );
					}
					else
					{
						SetComboboxSelection( IDC_COMBO_EMUL_DEVICES, uIndex );
					}
				}
				else
				{
					SetComboboxSelection( IDC_COMBO_EMUL_DEVICES, 0 );
				}
			}
		}
	}
	//
	////

	((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->GetWindowText( m_oCurrentSelection );

	////
	//
	m_uCurrentSelection = -1;

	for( m_uDev = 0; m_uDev < FPADIO_MAX_DEVICES; ++m_uDev )
	{
		if( m_paoSamples[ m_uDev ][ m_uValidSamples - 1 ].bValid )
		{
			fpadio_GetDeviceInfo( m_uDev, &m_oDeviceInfo );
			m_oString = m_oDeviceInfo.szName;
			if( m_oString == m_oCurrentSelection )
			{
				m_uCurrentSelection = m_uDev;
				break;
			}
		}
	}

	if( -1 == m_uCurrentSelection ) return;
	//
	////

	////
	//
	m_uAxes = 0;
	fpadio_GetDeviceInfo( m_uCurrentSelection, &m_oDeviceInfo );
	for( m_uInput = 0; m_uInput < m_oDeviceInfo.uInputs; ++m_uInput )
	{
		if( FPADIO_INPUT_DX_AXIS == m_oDeviceInfo.aeInputIDs[ m_uInput ] )
		{
			++m_uAxes;
		}
		else
		{
			break;
		}
	}
	//
	////

	////
	//
	if( 0 != strcmp( (LPCSTR)m_oCurrentSelection, m_oPreviousSelection ) )
	{
		m_oPreviousSelection = m_oCurrentSelection;

		for( m_uControl = IDC_COMBO_EMUL_XB_LEFT_STICK_X; m_uControl <= IDC_COMBO_EMUL_GC_DPAD_Y; ++m_uControl )
		{
			((CComboBox *)GetDlgItem( m_uControl ))->ResetContent();
			((CComboBox *)GetDlgItem( m_uControl ))->AddString( "Unassigned" );
			SetComboboxSelection( m_uControl, 0 );

			for( m_uInput = 0; m_uInput < m_uAxes; ++m_uInput )
			{
				m_oString.Format( "Axis %u", m_uInput + 1 );
				((CComboBox *)GetDlgItem( m_uControl ))->AddString( m_oString );
			}
		}

		for( m_uControl = IDC_COMBO_EMUL_XB_RESET; m_uControl <= IDC_COMBO_EMUL_GC_LEFT_TRIGGER_D; ++m_uControl )
		{
			((CComboBox *)GetDlgItem( m_uControl ))->ResetContent();
			((CComboBox *)GetDlgItem( m_uControl ))->AddString( "Unassigned" );
			SetComboboxSelection( m_uControl, 0 );

			for( m_uInput = 0; m_uInput < ( m_oDeviceInfo.uInputs - m_uAxes ); ++m_uInput )
			{
				m_oString.Format( "Button %u", m_uInput + 1 );
				((CComboBox *)GetDlgItem( m_uControl ))->AddString( m_oString );
			}
		}

		if( 0 == m_uDelay ) // Inital setup.
		{
			CSettings& oSettings = CSettings::GetCurrent();

			for( u32 uIndex = 0; uIndex < 36; ++uIndex )
			{
				SetComboboxSelection( IDC_COMBO_EMUL_XB_RESET + uIndex, oSettings.m_auInputEmulationMap[ uIndex ] );
			}
		}
	}
	//
	////

	////
	//
	m_bSet = FALSE;

	for( m_uInput = 0; ( ( ! m_bSet ) && ( m_uInput < m_oDeviceInfo.uInputs ) ); ++m_uInput )
	{
		if( ( +0.5f < m_paoSamples[ m_uCurrentSelection ][ m_uValidSamples - 1 ].afInputValues[ m_uInput ] ) ||
			( -0.5f > m_paoSamples[ m_uCurrentSelection ][ m_uValidSamples - 1 ].afInputValues[ m_uInput ] ) )
		{
			if( FPADIO_INPUT_DX_AXIS == m_oDeviceInfo.aeInputIDs[ m_uInput ] )
			{
				for( m_uControl = IDC_RADIO_EMUL_XB_LEFT_STICK_X; ( ! m_bSet ) && ( m_uControl <= IDC_RADIO_EMUL_GC_DPAD_Y ); ++m_uControl )
				{
					if( GetAttributeCheck( m_uControl ) )
					{
						SetComboboxSelection( m_uControl - ( IDC_RADIO_EMUL_XB_LEFT_STICK_X - IDC_COMBO_EMUL_XB_LEFT_STICK_X ), m_uInput + 1 );
						HideDropDownMenu( m_uControl - ( IDC_RADIO_EMUL_XB_LEFT_STICK_X - IDC_COMBO_EMUL_XB_LEFT_STICK_X ) );
						m_bSet = TRUE;
					}
				}
			}
			else // FPADIO_INPUT_DX_BUTTON
			{
				for( m_uControl = IDC_RADIO_EMUL_XB_RESET; ( ! m_bSet ) && ( m_uControl <= IDC_RADIO_EMUL_GC_LEFT_TRIGGER_D ); ++m_uControl )
				{
					if( GetAttributeCheck( m_uControl ) )
					{
						SetComboboxSelection( m_uControl - ( IDC_RADIO_EMUL_XB_RESET - IDC_COMBO_EMUL_XB_RESET ), m_uInput - m_uAxes + 1 );
						HideDropDownMenu( m_uControl - ( IDC_RADIO_EMUL_XB_RESET - IDC_COMBO_EMUL_XB_RESET ) );
						m_bSet = TRUE;
					}
				}
			}
		}
	}
	//
	////

	////
	//
	m_uDelay += _TIMER_DELAY;

	if( 200 <= m_uDelay )
	{
		m_uDelay = 1;
	}
	//
	////

	CDialog::OnTimer(nIDEvent);

} // CInputEmulation::OnTimer

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CInputEmulation::OnOK()
{
	KillTimer( _TIMER_CONTROL_SAMPLING );

	fpadio_Uninstall();

	if( CB_ERR == GetComboboxSelection( IDC_COMBO_EMUL_DEVICES ) ) return;

	CSettings& oSettings = CSettings::GetCurrent();

	((CComboBox *)GetDlgItem( IDC_COMBO_EMUL_DEVICES ))->GetWindowText( oSettings.m_sInputEmulationDevName );

	for( u32 uIndex = 0; uIndex < 36; ++uIndex )
	{
		oSettings.m_auInputEmulationMap[ uIndex ] = GetComboboxSelection( IDC_COMBO_EMUL_XB_RESET + uIndex );
	}

	oSettings.SaveCommonDataOutToFile();

	CDialog::OnOK();

} // CInputEmulation::OnOK

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CInputEmulation::GetAttributeCheck( int nID )
{
	return ((CButton *)GetDlgItem( nID ))->GetCheck();

} // CInputEmulation::GetAttributeCheck

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CInputEmulation::SetAttributeCheck( int nID, BOOL bVal )
{
	((CButton *)GetDlgItem( nID ))->SetCheck( bVal );

} // CInputEmulation::SetAttributeCheck

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CInputEmulation::SetComboboxSelection( int nID, u32 uSelection )
{
	((CComboBox *)GetDlgItem( nID ))->SetCurSel( uSelection );

} // CInputEmulation::SetComboboxSelection

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 CInputEmulation::GetComboboxSelection( int nID )
{
	return ((CComboBox *)GetDlgItem( nID ))->GetCurSel();

} // CInputEmulation::GetComboboxSelection

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CInputEmulation::HideDropDownMenu( int nID )
{
	((CComboBox *)GetDlgItem( nID ))->ShowDropDown( FALSE );

} // CInputEmulation::HideDropDownMenu

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CInputEmulation::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if( (             CBN_SETFOCUS == HIWORD( wParam ) ) &&
		( IDC_COMBO_EMUL_XB_RESET  <= LOWORD( wParam ) ) &&
		( IDC_COMBO_EMUL_GC_DPAD_Y >= LOWORD( wParam ) ) )
	{
		for( m_uControl = IDC_RADIO_EMUL_XB_RESET; m_uControl <= IDC_RADIO_EMUL_GC_DPAD_Y; ++m_uControl )
		{
			SetAttributeCheck( m_uControl, FALSE );
		}

		SetAttributeCheck( ( LOWORD( wParam ) + ( IDC_RADIO_EMUL_XB_RESET - IDC_COMBO_EMUL_XB_RESET ) ), TRUE );
	}

	if( (            CBN_KILLFOCUS == HIWORD( wParam ) ) &&
		( IDC_COMBO_EMUL_XB_RESET  <= LOWORD( wParam ) ) &&
		( IDC_COMBO_EMUL_GC_DPAD_Y >= LOWORD( wParam ) ) )
	{
		SetAttributeCheck( ( LOWORD( wParam ) + ( IDC_RADIO_EMUL_XB_RESET - IDC_COMBO_EMUL_XB_RESET ) ), FALSE );
	}

	return CDialog::OnCommand( wParam, lParam );

} // CInputEmulation::OnCommand

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
