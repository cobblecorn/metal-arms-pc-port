//////////////////////////////////////////////////////////////////////////////////////
// VidMode.cpp - 
//
// Author: Michael Starich   
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
// 06/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "VidMode.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _VARS_TO_CONTROLS	FALSE
#define _CONTROLS_TO_VARS	TRUE
#define DISABLE_FULLSCREEN TRUE

/////////////////////////////////////////////////////////////////////////////
// CVidMode dialog


CVidMode::CVidMode(CWnd* pParent /*=NULL*/)
	: CDialog(CVidMode::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVidMode)
	m_bFullscreen = FALSE;
	//}}AFX_DATA_INIT

	m_bModeSelected = GetVidModeFromRegistry( &m_CurrentVidWin );
}


void CVidMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVidMode)
	DDX_Control(pDX, IDC_COMBO_MODE, m_ComboMode);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_ComboDevice);
	DDX_Check(pDX, IDC_CHECK_FULLSCREEN, m_bFullscreen);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVidMode, CDialog)
	//{{AFX_MSG_MAP(CVidMode)
	ON_BN_CLICKED(IDC_RADIO_HARDWARE, OnRadioHardware)
	ON_BN_CLICKED(IDC_RADIO_SOFTWARE, OnRadioSoftware)
	ON_BN_CLICKED(IDC_RADIO_REFRAST, OnRadioRefrast)
	ON_BN_CLICKED(IDC_CHECK_FULLSCREEN, OnCheckFullscreen)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnSelchangeComboDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_MODE, OnSelchangeComboMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVidMode message handlers

BOOL CVidMode::GetVidMode( FVidWin_t *pDestVidWin ) {
	if( !m_bModeSelected ) {
		return FALSE;
	}

	pDestVidWin->VidDev = m_CurrentVidWin.VidDev;
	pDestVidWin->VidMode = m_CurrentVidWin.VidMode;
	pDestVidWin->bAllowPowerSuspend = TRUE;
	pDestVidWin->fUnitFSAA = 0.0f;
	pDestVidWin->nSwapInterval = 0;

	return TRUE;
}

BOOL CVidMode::GetVidModeFromRegistry( FVidWin_t *pDestVidWin ) {
	FVidWin_t VidWin;
	CSettings& Settings = CSettings::GetCurrent();

	strncpy( VidWin.VidDev.szName, Settings.GetDevName(), FVID_DEVNAME_LEN );
	if( !VidWin.VidDev.szName[0] ) {
		return FALSE;
	}
	VidWin.VidDev.nFlags = (u16)Settings.GetDevFlags();
	VidWin.VidDev.nOrdinal = (u8)Settings.GetDevOrdinal();
	VidWin.VidDev.nRenderer = (u8)Settings.GetDevRenderer();
	VidWin.VidMode.nFlags = (u8)Settings.GetModeFlags();
	VidWin.VidMode.nColorBits = (u8)Settings.GetModeColorBits();
	if( !VidWin.VidMode.nColorBits ) {
		return FALSE;
	}
	VidWin.VidMode.nDepthBits = (u8)Settings.GetModeDepthBits();
	VidWin.VidMode.nStencilBits = (u8)Settings.GetModeStencilBits();
	VidWin.VidMode.nPixelsAcross = (u16)Settings.GetModePixelsAcross();
	if( !VidWin.VidMode.nPixelsAcross ) {
		return FALSE;
	}
	VidWin.VidMode.nPixelsDown = (u16)Settings.GetModePixelsDown();
	if( !VidWin.VidMode.nPixelsDown ) {
		return FALSE;
	}
	VidWin.nSwapInterval = (u32)Settings.GetSwapInterval();
	VidWin.fUnitFSAA = Settings.GetUnitFSAA();

	*pDestVidWin = VidWin;

	return TRUE;
}

BOOL CVidMode::StoreVidModeIntoRegistry( const FVidWin_t *pVidWin ) {
	CSettings& Settings = CSettings::GetCurrent();

	UpdateData( _CONTROLS_TO_VARS );

	Settings.m_sDevName = pVidWin->VidDev.szName;
	Settings.m_nDevFlags = pVidWin->VidDev.nFlags;
	Settings.m_nDevOrdinal = pVidWin->VidDev.nOrdinal;
	Settings.m_nDevRenderer = pVidWin->VidDev.nRenderer;
	Settings.m_nModeFlags = pVidWin->VidMode.nFlags;
	Settings.m_nModeColorBits = pVidWin->VidMode.nColorBits;
	Settings.m_nModeDepthBits = pVidWin->VidMode.nDepthBits;
	Settings.m_nModeStencilBits = pVidWin->VidMode.nStencilBits;
	Settings.m_nModePixelsAcross = pVidWin->VidMode.nPixelsAcross;
	Settings.m_nModePixelsDown = pVidWin->VidMode.nPixelsDown;
	Settings.m_nSwapInterval = pVidWin->nSwapInterval;
	Settings.m_fUnitFSAA = pVidWin->fUnitFSAA;

	Settings.SaveCommonDataOutToFile();

	return TRUE;
}

BOOL CVidMode::OnInitDialog() {
	CDialog::OnInitDialog();

	m_bHighlightedMode = FALSE;

	if( m_bModeSelected ) {
		m_nDeviceType = (FVidRenderer_t)m_CurrentVidWin.VidDev.nRenderer;
		m_bFullscreen = (m_CurrentVidWin.VidMode.nFlags & FVID_MODEFLAG_WINDOWED) ? FALSE : TRUE;
	} else {
		m_nDeviceType = FVID_RENDERER_HARDWARE;
		m_bFullscreen = TRUE;
	}
	#if DISABLE_FULLSCREEN
		m_bFullscreen = FALSE;
	#endif

	((CButton *)GetDlgItem( IDC_RADIO_HARDWARE ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_RADIO_SOFTWARE ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_RADIO_REFRAST ))->SetCheck( FALSE );
	switch( m_nDeviceType ) {
	case FVID_RENDERER_HARDWARE:
		((CButton *)GetDlgItem( IDC_RADIO_HARDWARE ))->SetCheck( TRUE );
		break;

	case FVID_RENDERER_SOFTWARE:
		((CButton *)GetDlgItem( IDC_RADIO_SOFTWARE ))->SetCheck( TRUE );
		break;

	case FVID_RENDERER_EMULATION:
		((CButton *)GetDlgItem( IDC_RADIO_REFRAST ))->SetCheck( TRUE );
		break;

	default:
		FASSERT_NOW;
	}
	((CButton *)GetDlgItem( IDC_CHECK_FULLSCREEN ))->SetCheck( m_bFullscreen );

	UpdateData( _VARS_TO_CONTROLS );

	Enumerate();

	if( m_bModeSelected ) {
		SelectDefaultsInCombos();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVidMode::Enumerate() {
	FVidRenderer_t nCurrentEnumeratedDevType;
	BOOL bMustEnumerate = FALSE;

	if( !fvid_HasEnumerated( &nCurrentEnumeratedDevType ) ) {
		// Haven't enumerated...
		bMustEnumerate = TRUE;
	} else {
		if( nCurrentEnumeratedDevType != m_nDeviceType ) {
			bMustEnumerate = TRUE;
		}
	}

	if( bMustEnumerate ) {
		((CButton *)GetDlgItem( IDC_CHECK_FULLSCREEN ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_RADIO_HARDWARE ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_RADIO_SOFTWARE ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_RADIO_REFRAST ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDOK ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( FALSE );
		m_ComboDevice.EnableWindow( FALSE );
		m_ComboMode.EnableWindow( FALSE );

		fvid_Enumerate( m_nDeviceType );

#if DISABLE_FULLSCREEN
		((CButton *)GetDlgItem( IDC_CHECK_FULLSCREEN ))->EnableWindow( TRUE );
#endif
		((CButton *)GetDlgItem( IDC_RADIO_HARDWARE ))->EnableWindow( TRUE );
		((CButton *)GetDlgItem( IDC_RADIO_SOFTWARE ))->EnableWindow( TRUE );
		((CButton *)GetDlgItem( IDC_RADIO_REFRAST ))->EnableWindow( TRUE );
		((CButton *)GetDlgItem( IDOK ))->EnableWindow( m_bHighlightedMode );
		((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( TRUE );
		m_ComboDevice.EnableWindow( TRUE );
		m_ComboMode.EnableWindow( TRUE );
	}

	u32 nDevCount, nDevIndex, nDefaultDevIndex;
	const FVidDev_t *pDev;
	CString sDevice;
	int nComboIndex;

	m_ComboDevice.ResetContent();
	m_ComboMode.ResetContent();

	nDevCount = fvid_GetDeviceCount();

	if( nDevCount ) {
		nDefaultDevIndex = 0;
		for( nDevIndex=0; nDevIndex<nDevCount; nDevIndex++ ) {
			pDev = fvid_GetDeviceInfo( nDevIndex );

			sDevice.Format( "%s%s", pDev->szName, (pDev->nFlags & FVID_DEVFLAG_HW_TNL) ? " (HWTnL)" : "" );

			nComboIndex = m_ComboDevice.AddString( (cchar *)sDevice );
			m_ComboDevice.SetItemData( nComboIndex, nDevIndex );

			if( pDev->nFlags & FVID_DEVFLAG_HW_TNL ) {
				if( nDefaultDevIndex == 0 ) {
					nDefaultDevIndex = nDevIndex;
				}
			}
		}

		pDev = fvid_GetDeviceInfo( nDefaultDevIndex );
		m_ComboDevice.SelectString( -1, pDev->szName );
	}

	UpdateData( _CONTROLS_TO_VARS );

	FillComboModeList();
}

void CVidMode::OnRadioHardware() {
	m_nDeviceType = FVID_RENDERER_HARDWARE;
	Enumerate();
}

void CVidMode::OnRadioSoftware() {
	m_nDeviceType = FVID_RENDERER_SOFTWARE;
	Enumerate();
}

void CVidMode::OnRadioRefrast() {
	m_nDeviceType = FVID_RENDERER_EMULATION;
	Enumerate();
}

void CVidMode::OnCancel() {
	CDialog::OnCancel();
}

void CVidMode::OnOK() {
	UpdateData( _VARS_TO_CONTROLS );

	ZeroMemory( &m_CurrentVidWin, sizeof(m_CurrentVidWin) );
	m_CurrentVidWin.VidDev = m_HighlightedVidDev;
	m_CurrentVidWin.VidMode = m_HighlightedVidMode;
	m_CurrentVidWin.bAllowPowerSuspend = TRUE;
	m_CurrentVidWin.fUnitFSAA = 0.0f;
	m_CurrentVidWin.nSwapInterval = 0;

	m_bModeSelected = TRUE;

	if( !StoreVidModeIntoRegistry( &m_CurrentVidWin ) ){
		CString sTitle;
		sTitle = CSettings::GetApplicationName();
		sTitle += " Video Mode";
		MessageBox( "Could not update registry.", sTitle );
	}

	CDialog::OnOK();
}

void CVidMode::OnCheckFullscreen() {
	m_bFullscreen = ((CButton *)GetDlgItem( IDC_CHECK_FULLSCREEN ))->GetCheck();
	FillComboModeList();
}

void CVidMode::FillComboModeList() {
	int nCurrentComboDevIndex, nComboModeIndex, nDefaultComboModeIndex;
	u32 nDevIndex, nModeCount, nModeIndex, nLargestDepthStencilBits, nDefaultIndexBits, nDefaultIndexPixelsAcross;
	const FVidMode_t *pMode;
	CString sMode;

	m_ComboMode.ResetContent();

	nCurrentComboDevIndex = m_ComboDevice.GetCurSel();
	if( nCurrentComboDevIndex != CB_ERR ) {
		nDevIndex = m_ComboDevice.GetItemData( nCurrentComboDevIndex );

		nDefaultComboModeIndex = 0;
		nDefaultIndexBits = 0;
		nDefaultIndexPixelsAcross = 0;

		nModeCount = fvid_GetModeCount( nDevIndex );
		if( nModeCount ) {
			nLargestDepthStencilBits = 0;
			for( nModeIndex=0; nModeIndex<nModeCount; nModeIndex++ ) {
				pMode = fvid_GetModeInfo( nDevIndex, nModeIndex );
				if( ((u32)pMode->nDepthBits + (u32)pMode->nStencilBits) > nLargestDepthStencilBits ) {
					nLargestDepthStencilBits = pMode->nDepthBits + pMode->nStencilBits;
				}
			}

			for( nModeIndex=0; nModeIndex<nModeCount; nModeIndex++ ) {
				pMode = fvid_GetModeInfo( nDevIndex, nModeIndex );

				if( !(pMode->nFlags & FVID_MODEFLAG_WINDOWED) != !!m_bFullscreen ) {
					continue;
				}
				if( ((u32)pMode->nDepthBits + (u32)pMode->nStencilBits) < nLargestDepthStencilBits ) {
					continue;
				}

				sMode.Format( "%ub %ux%u (depth=%ub, stencil=%ub)", pMode->nColorBits, pMode->nPixelsAcross, pMode->nPixelsDown, pMode->nDepthBits, pMode->nStencilBits );

				nComboModeIndex = m_ComboMode.AddString( (cchar *)sMode );
				m_ComboMode.SetItemData( nComboModeIndex, nModeIndex );

				if( pMode->nColorBits > nDefaultIndexBits ) {
					nDefaultIndexBits = pMode->nColorBits;
					nDefaultIndexPixelsAcross = pMode->nPixelsAcross;
					nDefaultComboModeIndex = nComboModeIndex;
				} else if( pMode->nPixelsAcross>nDefaultIndexPixelsAcross && pMode->nPixelsAcross<=800 ) {
					nDefaultIndexPixelsAcross = pMode->nPixelsAcross;
					nDefaultComboModeIndex = nComboModeIndex;
				}
			}

			m_ComboMode.SetCurSel( nDefaultComboModeIndex );
		}
	}

	UpdateData( _CONTROLS_TO_VARS );

	OnSelchangeComboMode();
}

void CVidMode::OnSelchangeComboDevice() {
	FillComboModeList();
}

void CVidMode::OnSelchangeComboMode() {
	int nComboDevIndex, nComboModeIndex;
	u32 nDevIndex, nModeIndex;

	UpdateData( _CONTROLS_TO_VARS );

	nComboDevIndex = m_ComboDevice.GetCurSel();
	if( nComboDevIndex != CB_ERR ) {
		nComboModeIndex = m_ComboMode.GetCurSel();
		if( nComboModeIndex != CB_ERR ) {
			nDevIndex = m_ComboDevice.GetItemData( nComboDevIndex );
			nModeIndex = m_ComboMode.GetItemData( nComboModeIndex );

			m_bHighlightedMode = TRUE;
			m_HighlightedVidDev = *fvid_GetDeviceInfo( nDevIndex );
			m_HighlightedVidMode = *fvid_GetModeInfo( nDevIndex, nModeIndex );
			((CButton *)GetDlgItem( IDOK ))->EnableWindow( TRUE );
			return;
		}
	}

	((CButton *)GetDlgItem( IDOK ))->EnableWindow( FALSE );
	m_bHighlightedMode = FALSE;

	UpdateData( _VARS_TO_CONTROLS );
}

BOOL CVidMode::AreDevsIdentical( const FVidDev_t *pDev1, const FVidDev_t *pDev2 ) {
	if( strcmp( pDev1->szName, pDev2->szName ) ) {
		return FALSE;
	}

	if( pDev1->nFlags != pDev2->nFlags ) {
		return FALSE;
	}

	if( pDev1->nOrdinal != pDev2->nOrdinal ) {
		return FALSE;
	}

	if( pDev1->nRenderer != pDev2->nRenderer ) {
		return FALSE;
	}

	return TRUE;
}

BOOL CVidMode::AreModesIdentical( const FVidMode_t *pMode1, const FVidMode_t *pMode2 ) {
	if( pMode1->nColorBits != pMode2->nColorBits ) {
		return FALSE;
	}

	if( pMode1->nDepthBits != pMode2->nDepthBits ) {
		return FALSE;
	}

	if( pMode1->nFlags != pMode2->nFlags ) {
		return FALSE;
	}

	if( pMode1->nPixelsAcross != pMode2->nPixelsAcross ) {
		return FALSE;
	}

	if( pMode1->nPixelsDown != pMode2->nPixelsDown ) {
		return FALSE;
	}

	if( pMode1->nStencilBits != pMode2->nStencilBits ) {
		return FALSE;
	}

	return TRUE;
}

void CVidMode::SelectDefaultsInCombos() {
	u32 nDevIndex, nDevCount, nModeIndex, nModeCount, nItemDataDevIndex, nItemDataModeIndex;
	const FVidDev_t *pDev;
	const FVidMode_t *pMode;
	int nComboDevCount, nComboModeCount, nComboDevIndex, nComboModeIndex;

	nDevCount = fvid_GetDeviceCount();

	nComboDevCount = m_ComboDevice.GetCount();
	nComboModeCount = m_ComboMode.GetCount();

	for( nDevIndex=0; nDevIndex<nDevCount; nDevIndex++ ) {
		pDev = fvid_GetDeviceInfo( nDevIndex );

		if( !AreDevsIdentical( pDev, &m_CurrentVidWin.VidDev ) ) {
			continue;
		}

		for( nComboDevIndex=0; nComboDevIndex<nComboDevCount; nComboDevIndex++ ) {
			nItemDataDevIndex = m_ComboDevice.GetItemData( nComboDevIndex );
			if( nItemDataDevIndex == nDevIndex ) {
				break;
			}
		}
		if( nComboDevIndex == nComboDevCount ) {
			continue;
		}

		nModeCount = fvid_GetModeCount( nDevIndex );

		for( nModeIndex=0; nModeIndex<nModeCount; nModeIndex++ ) {
			pMode = fvid_GetModeInfo( nDevIndex, nModeIndex );

			if( AreModesIdentical( pMode, &m_CurrentVidWin.VidMode ) ) {
				for( nComboModeIndex=0; nComboModeIndex<nComboModeCount; nComboModeIndex++ ) {
					nItemDataModeIndex = m_ComboMode.GetItemData( nComboModeIndex );
					if( nItemDataModeIndex == nModeIndex ) {
						// Found our mode...

						m_ComboDevice.SetCurSel( nComboDevIndex );
						m_ComboMode.SetCurSel( nComboModeIndex );
						return;
					}
				}
			}
		}
	}
	UpdateData( _CONTROLS_TO_VARS );
}


