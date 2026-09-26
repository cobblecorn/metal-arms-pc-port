// GlobalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "GlobalDlg.h"
#include "SpitDlg.h"
#include "PickAsset.h"
#include "Settings.h"
#include "fclib.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CGlobalDlg dialog

IMPLEMENT_DYNCREATE(CGlobalDlg, CDialog)

CGlobalDlg::CGlobalDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalDlg::IDD, pParent)
	, m_sSoundName(_T(""))
	, m_sCoronaTexture(_T(""))
	, m_fCoronaScale(0)
	, m_bSortParticleGroup(FALSE)
{
	//{{AFX_DATA_INIT(CGlobalDlg)
	m_bBurstForever = FALSE;
	m_bDirectional = FALSE;
	m_bLight = FALSE;
	m_bZScale = FALSE;
	m_bIgnoreWind = FALSE;
	m_bMultiSprites = FALSE;
	m_bNoFog = FALSE;
	m_bOneShot = FALSE;
	m_bPixelSize = FALSE;
	m_nDrawMode = -1;
	m_bRandFinalColor = FALSE;
	m_bRandInitColor = FALSE;
	m_bSampleLights = FALSE;
	m_bSplitOnImpact = FALSE;
	m_bZBuffer = FALSE;
	m_sTextureName = _T("");
	m_bEnableTextures = FALSE;
	m_bUseBubbleForce = FALSE;
	m_bAddPosJitter = FALSE;
	m_bCloseSpriteGaps = FALSE;
	m_bPlaySound = FALSE;
	m_bPerPixelLight = FALSE;
	m_bAutoCalcLightIntensity = FALSE;
	m_bCoronaOn = FALSE;
	m_bPerspectiveScaleCorona = FALSE;
	m_bSpikeColor = FALSE;
	m_bSpikeAlpha = FALSE;
	m_bSpikeScale = FALSE;
	//}}AFX_DATA_INIT
}


void CGlobalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalDlg)
	DDX_Control(pDX, IDC_EDIT_MOTIF_INDEX, m_ctrlMotifIndex);
	DDX_Check(pDX, IDC_BURST_FOREVER, m_bBurstForever);
	DDX_Check(pDX, IDC_DIRECTIONAL_EMISSION, m_bDirectional);
	DDX_Check(pDX, IDC_EMIT_LIGHT, m_bLight);
	DDX_Check(pDX, IDC_GROUP_Z_SCALE, m_bZScale);
	DDX_Check(pDX, IDC_IGNORE_WIND, m_bIgnoreWind);
	DDX_Check(pDX, IDC_MULTI_SPRITES, m_bMultiSprites);
	DDX_Check(pDX, IDC_NO_FOG, m_bNoFog);
	DDX_Check(pDX, IDC_ONE_SHOT, m_bOneShot);
	DDX_Check(pDX, IDC_PIXEL_SIZE, m_bPixelSize);
	DDX_Radio(pDX, IDC_RADIO_MODULATE_MODE, m_nDrawMode);
	DDX_Check(pDX, IDC_RANDOM_FINAL_COLOR, m_bRandFinalColor);
	DDX_Check(pDX, IDC_RANDOM_INIT_COLOR, m_bRandInitColor);
	DDX_Check(pDX, IDC_SAMPLE_LIGHTS, m_bSampleLights);
	DDX_Check(pDX, IDC_SPLIT_ON_IMPACT, m_bSplitOnImpact);
	DDX_Check(pDX, IDC_Z_BUFFER, m_bZBuffer);
	DDX_Text(pDX, IDC_TEXTURE_NAME, m_sTextureName);
	DDX_Check(pDX, IDC_ENABLE_TEXTURES, m_bEnableTextures);
	DDX_Check(pDX, IDC_USE_BUBBLE_FORCE, m_bUseBubbleForce);
	DDX_Check(pDX, IDC_JITTER_POS, m_bAddPosJitter);
	DDX_Check(pDX, IDC_CLOSE_SPRITES_GAP, m_bCloseSpriteGaps);
	DDX_Check(pDX, IDC_ENABLE_SOUND, m_bPlaySound);
	DDX_Check(pDX, IDC_PER_PIXEL_LIGHT_ON, m_bPerPixelLight);
	DDX_Check(pDX, IDC_AUTO_CALCULATE_LIGHT_INTENSITY_ON, m_bAutoCalcLightIntensity);
	DDX_Check(pDX, IDC_CORONA_ON, m_bCoronaOn);
	DDX_Check(pDX, IDC_WORLD_SCALE_CORONA_ON, m_bPerspectiveScaleCorona);
	DDX_Check(pDX, IDC_SPIKE_COLOR_ON, m_bSpikeColor);
	DDX_Check(pDX, IDC_SPIKE_ALPHA_ON, m_bSpikeAlpha);
	DDX_Check(pDX, IDC_SPIKE_SCALE_ON, m_bSpikeScale);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_SCALE_SAMPLED_RED, m_ctrlScaleRedSamples, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_SCALE_SAMPLED_GREEN, m_ctrlScaleGreenSamples, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_SCALE_SAMPLED_BLUE, m_ctrlScaleBlueSamples, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_SAMPLES_PER_SEC, m_ctrlSamplesPerSec, 0);
	DDX_Text(pDX, IDC_SOUND_NAME, m_sSoundName);
	DDV_MaxChars(pDX, m_sSoundName, 15);
	DDX_Text(pDX, IDC_CORONA_TEXTURE_NAME, m_sCoronaTexture);
	DDV_MaxChars(pDX, m_sCoronaTexture, 15);
	DDX_Text(pDX, IDC_EDIT_CORONA_SCALE, m_fCoronaScale);
	//DDV_MinMaxFloat(pDX, m_fCoronaScale, 0.1f, 5.0f);
	DDX_Check(pDX, IDC_SORT, m_bSortParticleGroup);
}


BEGIN_MESSAGE_MAP(CGlobalDlg, CDialog)
	//{{AFX_MSG_MAP(CGlobalDlg)
	ON_BN_CLICKED(IDC_BURST_FOREVER, OnBurstForever)
	ON_BN_CLICKED(IDC_DIRECTIONAL_EMISSION, OnDirectionalEmission)
	ON_BN_CLICKED(IDC_EMIT_LIGHT, OnEmitLight)
	ON_BN_CLICKED(IDC_GROUP_Z_SCALE, OnGroupZScale)
	ON_BN_CLICKED(IDC_IGNORE_WIND, OnIgnoreWind)
	ON_BN_CLICKED(IDC_MULTI_SPRITES, OnMultiSprites)
	ON_BN_CLICKED(IDC_NO_FOG, OnNoFog)
	ON_BN_CLICKED(IDC_ONE_SHOT, OnOneShot)
	ON_BN_CLICKED(IDC_PIXEL_SIZE, OnPixelSize)
	ON_BN_CLICKED(IDC_RADIO_ADD_MODE, OnRadioAddMode)
	ON_BN_CLICKED(IDC_RADIO_MODULATE_MODE, OnRadioModulateMode)
	ON_BN_CLICKED(IDC_RANDOM_FINAL_COLOR, OnRandomFinalColor)
	ON_BN_CLICKED(IDC_RANDOM_INIT_COLOR, OnRandomInitColor)
	ON_BN_CLICKED(IDC_SAMPLE_LIGHTS, OnSampleLights)
	ON_BN_CLICKED(IDC_SPLIT_ON_IMPACT, OnSplitOnImpact)
	ON_BN_CLICKED(IDC_Z_BUFFER, OnZBuffer)
	ON_BN_CLICKED(IDC_ENABLE_TEXTURES, OnEnableTextures)
	ON_BN_CLICKED(IDC_CHOOSE_TEXTURE, OnChooseTexture)
	ON_EN_KILLFOCUS(IDC_EDIT_MOTIF_INDEX, OnKillfocusEditMotifIndex)
	ON_EN_KILLFOCUS(IDC_EDIT_SAMPLES_PER_SEC, OnKillfocusEditSamplesPerSec)
	ON_EN_CHANGE(IDC_EDIT_SAMPLES_PER_SEC, OnChangeEditSamplesPerSec)
	ON_EN_KILLFOCUS(IDC_EDIT_SCALE_SAMPLED_BLUE, OnKillfocusEditScaleSampledBlue)
	ON_EN_CHANGE(IDC_EDIT_SCALE_SAMPLED_BLUE, OnChangeEditScaleSampledBlue)
	ON_EN_KILLFOCUS(IDC_EDIT_SCALE_SAMPLED_GREEN, OnKillfocusEditScaleSampledGreen)
	ON_EN_CHANGE(IDC_EDIT_SCALE_SAMPLED_GREEN, OnChangeEditScaleSampledGreen)
	ON_EN_KILLFOCUS(IDC_EDIT_SCALE_SAMPLED_RED, OnKillfocusEditScaleSampledRed)
	ON_EN_CHANGE(IDC_EDIT_SCALE_SAMPLED_RED, OnChangeEditScaleSampledRed)
	ON_BN_CLICKED(IDC_JITTER_POS, OnJitterPos)
	ON_BN_CLICKED(IDC_USE_BUBBLE_FORCE, OnUseBubbleForce)
	ON_BN_CLICKED(IDC_CLOSE_SPRITES_GAP, OnCloseSpriteGaps)
	ON_BN_CLICKED(IDC_ENABLE_SOUND, OnPlaySound)
	ON_BN_CLICKED(IDC_PER_PIXEL_LIGHT_ON, OnPerPixelLight)
	ON_BN_CLICKED(IDC_AUTO_CALCULATE_LIGHT_INTENSITY_ON, OnAutoCalcLightIntensity)
	ON_BN_CLICKED(IDC_CORONA_ON, OnCoronaOn)
	ON_BN_CLICKED(IDC_WORLD_SCALE_CORONA_ON, OnPerspectiveScaleCorona)
	ON_BN_CLICKED(IDC_SPIKE_COLOR_ON, OnSpikeColor)
	ON_BN_CLICKED(IDC_SPIKE_ALPHA_ON, OnSpikeAlpha)
	ON_BN_CLICKED(IDC_SPIKE_SCALE_ON, OnSpikeScale)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
	ON_BN_CLICKED(IDC_CHOOSE_CORONA_TEXTURE, OnBnClickedChooseCoronaTexture)
	ON_BN_CLICKED(IDC_CLEAR_CORONA_TEXTURE, OnBnClickedClearCoronaTexture)
	ON_EN_KILLFOCUS(IDC_EDIT_CORONA_SCALE, OnEnKillfocusEditCoronaScale)
	ON_EN_KILLFOCUS(IDC_SOUND_NAME, OnEnKillfocusSoundName)
	ON_BN_CLICKED(IDC_SORT, OnBnClickedSort)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalDlg message handlers

BOOL CGlobalDlg::PreTranslateMessage(MSG* pMsg) {
	// Detect the Enter (Return) key, and pass the message to the dialog.
	// This disables the "default button" functionality normally
	// found in dialogs and property sheets, and allows the user to hit
	// Enter to "commit" the changes (s)he is making to an edit field.
	if( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN ) {
		CWnd* pControl = GetFocus();
		if( pControl ) {
			char pszString[256];
			if( ::GetClassName( pControl->m_hWnd, pszString, 256 ) ) {
				// see if we have an edit control
				CString s;
				s = "edit";
				if( !s.CompareNoCase( pszString ) ) {
					// the strings matched, we have an edit box, highlight the entire line and kill focus
					
					// Tell the dialog that Enter was pressed, and pass the ID
					// of the edit control which has the focus.
					PostMessage(WM_COMMAND, MAKELONG(pControl->GetDlgCtrlID(), EN_KILLFOCUS), (LPARAM) pControl->m_hWnd);
					
					// Highlight all text in the edit control.
					pControl->PostMessage( EM_SETSEL, 0, -1 );

					return TRUE;
				} 
			}
		}
	}
	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the dialog, do nothing
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CGlobalDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_ctrlScaleRedSamples.SetRange( 0, 100 );

	m_ctrlScaleGreenSamples.SetRange( 0, 100 );
	
	m_ctrlScaleBlueSamples.SetRange( 0, 100 );
	
	m_ctrlSamplesPerSec.SetRange( 1, 20 );
	
	m_ctrlMotifIndex.SetMaxWholeDigits( 3 );
	m_ctrlMotifIndex.AllowNegative( FALSE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGlobalDlg::OnOK() {
	//OnCancel();
}

void CGlobalDlg::OnBurstForever() {

	UpdateData( CONTROLS_TO_VARS );	

	if( m_bBurstForever ) {
		m_GameData.nFlags |= FPARTICLE_DEF_FLAGS_BURST_FOREVER;
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_ONE_SHOT;

		m_bOneShot = FALSE;
		UpdateData( VARS_TO_CONTROLS );
	} else {
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_BURST_FOREVER;
	}
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CGlobalDlg::OnDirectionalEmission() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_DIRECTIONAL, m_bDirectional );	
}

void CGlobalDlg::OnEmitLight() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_EMIT_LIGHT, m_bLight );
}

void CGlobalDlg::OnGroupZScale() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_GROUP_ZSCALE, m_bZScale );
}

void CGlobalDlg::OnIgnoreWind() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_IGNORE_WIND, m_bIgnoreWind );
}

void CGlobalDlg::OnMultiSprites() {
	
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_MULTIPLE_SPRITES, m_bMultiSprites );
}

void CGlobalDlg::OnCloseSpriteGaps() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_CLOSE_SPRITE_GAPS, m_bCloseSpriteGaps );	
}

void CGlobalDlg::OnNoFog() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_NO_FOG, m_bNoFog );
}

void CGlobalDlg::OnOneShot() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	if( m_bOneShot ) {
		m_GameData.nFlags |= FPARTICLE_DEF_FLAGS_ONE_SHOT;
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_BURST_FOREVER;

		m_bBurstForever = FALSE;
		UpdateData( VARS_TO_CONTROLS );
	} else {
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_ONE_SHOT;
	}
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CGlobalDlg::OnPixelSize() {
	
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_SIZE_IN_PIXELS, m_bPixelSize );
}

void CGlobalDlg::OnRadioAddMode() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	switch( m_nDrawMode ) {
	case 0:
		// modulate
		m_GameData.nFlags |= FPARTICLE_DEF_FLAGS_DRAW_MODE_MODULATE;
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_DRAW_MODE_ADD;
		break;
	case 1:
		// add
		m_GameData.nFlags |= FPARTICLE_DEF_FLAGS_DRAW_MODE_ADD;
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_DRAW_MODE_MODULATE;
		break;
	}
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnRadioModulateMode() {
	
	UpdateData( CONTROLS_TO_VARS );	

	switch( m_nDrawMode ) {
	case 0:
		// modulate
		m_GameData.nFlags |= FPARTICLE_DEF_FLAGS_DRAW_MODE_MODULATE;
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_DRAW_MODE_ADD;
		break;
	case 1:
		// add
		m_GameData.nFlags |= FPARTICLE_DEF_FLAGS_DRAW_MODE_ADD;
		m_GameData.nFlags &= ~FPARTICLE_DEF_FLAGS_DRAW_MODE_MODULATE;
		break;
	}
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnRandomFinalColor() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_RANDOM_FINAL_COLOR, m_bRandFinalColor );
}

void CGlobalDlg::OnRandomInitColor() {
	
	UpdateData( CONTROLS_TO_VARS );	
	
	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_RANDOM_INIT_COLOR, m_bRandInitColor );
}

void CGlobalDlg::OnSampleLights() {
	
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_SAMPLE_LIGHTS, m_bSampleLights );
}

void CGlobalDlg::OnSplitOnImpact() {
	
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_SPLIT_ON_IMPACT, m_bSplitOnImpact );	
}

void CGlobalDlg::OnZBuffer() {
	
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_Z_BUFFER, m_bZBuffer );
}

void CGlobalDlg::SetData( CGlobalDlg_Data_t *pData ) {
	CString sValue;

	m_bLight = ( pData->nFlags & FPARTICLE_DEF_FLAGS_EMIT_LIGHT ) ? TRUE : FALSE;
	
	m_bOneShot = ( pData->nFlags & FPARTICLE_DEF_FLAGS_ONE_SHOT ) ? TRUE : FALSE;

	m_bSplitOnImpact = ( pData->nFlags & FPARTICLE_DEF_FLAGS_SPLIT_ON_IMPACT ) ? TRUE : FALSE;

	m_bZScale = ( pData->nFlags & FPARTICLE_DEF_FLAGS_GROUP_ZSCALE ) ? TRUE : FALSE;

	m_bEnableTextures = ( pData->nFlags & FPARTICLE_DEF_FLAGS_TEXTURED ) ? TRUE : FALSE;

	m_bIgnoreWind = ( pData->nFlags & FPARTICLE_DEF_FLAGS_IGNORE_WIND ) ? TRUE : FALSE;

	m_bDirectional = ( pData->nFlags & FPARTICLE_DEF_FLAGS_DIRECTIONAL ) ? TRUE : FALSE;

	m_nDrawMode = ( pData->nFlags & FPARTICLE_DEF_FLAGS_DRAW_MODE_MODULATE ) ? 0 : 1;

	m_bNoFog = ( pData->nFlags & FPARTICLE_DEF_FLAGS_NO_FOG ) ? TRUE : FALSE;
	
	m_bZBuffer = ( pData->nFlags & FPARTICLE_DEF_FLAGS_Z_BUFFER ) ? TRUE : FALSE;
	
	m_bSortParticleGroup = ( pData->nFlags & FPARTICLE_DEF_FLAGS_SORT ) ? TRUE : FALSE;
	
	m_bPixelSize = ( pData->nFlags & FPARTICLE_DEF_FLAGS_SIZE_IN_PIXELS ) ? TRUE : FALSE;
	
	m_bSampleLights = ( pData->nFlags & FPARTICLE_DEF_FLAGS_SAMPLE_LIGHTS ) ? TRUE : FALSE;
	
	m_bMultiSprites = ( pData->nFlags & FPARTICLE_DEF_FLAGS_MULTIPLE_SPRITES ) ? TRUE : FALSE;
	
	m_bBurstForever = ( pData->nFlags & FPARTICLE_DEF_FLAGS_BURST_FOREVER ) ? TRUE : FALSE;
	
	m_bRandInitColor = ( pData->nFlags & FPARTICLE_DEF_FLAGS_RANDOM_INIT_COLOR ) ? TRUE : FALSE;
	
	m_bRandFinalColor = ( pData->nFlags & FPARTICLE_DEF_FLAGS_RANDOM_FINAL_COLOR ) ? TRUE : FALSE;

	m_bUseBubbleForce = ( pData->nFlags & FPARTICLE_DEF_FLAGS_BUBBLE_MOTION ) ? TRUE : FALSE;

	m_bAddPosJitter = ( pData->nFlags & FPARTICLE_DEF_FLAGS_JITTER_POS ) ? TRUE : FALSE;

	m_bCloseSpriteGaps = ( pData->nFlags & FPARTICLE_DEF_FLAGS_CLOSE_SPRITE_GAPS ) ? TRUE : FALSE;
	
	m_bPlaySound = ( pData->nFlags & FPARTICLE_DEF_FLAGS_PLAY_SOUND ) ? TRUE : FALSE;
	
	m_bSpikeColor = ( pData->nFlags & FPARTICLE_DEF_FLAGS_SPIKE_COLOR ) ? TRUE : FALSE;
	
	m_bSpikeAlpha = ( pData->nFlags & FPARTICLE_DEF_FLAGS_SPIKE_ALPHA ) ? TRUE : FALSE; 
	
	m_bSpikeScale = ( pData->nFlags & FPARTICLE_DEF_FLAGS_SPIKE_SCALE ) ? TRUE : FALSE;
	
	m_bPerPixelLight = ( pData->nFlags2 & FPARTICLE_DEF_FLAG2_PER_PIXEL_LIGHT ) ? TRUE : FALSE;
	
	m_bAutoCalcLightIntensity = ( pData->nFlags2 & FPARTICLE_DEF_FLAG2_AUTO_CALC_LIGHT_INTENSITY ) ? TRUE : FALSE;
	
	m_bCoronaOn = ( pData->nFlags2 & FPARTICLE_DEF_FLAG2_LIGHT_CORONA ) ? TRUE : FALSE;
	
	m_bPerspectiveScaleCorona = ( pData->nFlags2 & FPARTICLE_DEF_FLAG2_WORLD_SPACE_CORONA_SCALE ) ? TRUE : FALSE;
	
	m_sTextureName = pData->szTextureName;

	m_sCoronaTexture = pData->szCoronaTextureName;

	m_sSoundName = pData->szSoundFxName;

	m_fCoronaScale = pData->fCoronaScale;

	m_ctrlScaleRedSamples.SetPos( (int)AccuCast( pData->SampledLightScale.x * 100.0f ) );
	m_ctrlScaleGreenSamples.SetPos( (int)AccuCast( pData->SampledLightScale.y * 100.0f ) );
	m_ctrlScaleBlueSamples.SetPos( (int)AccuCast( pData->SampledLightScale.z * 100.0f ) );
	
	if( pData->fSecsBetweenLightSamples < (1.0f/20.0f) ) {
		m_ctrlSamplesPerSec.SetPos( 1 );
	} else {
		m_ctrlSamplesPerSec.SetPos( (int)AccuCast( 1.0f / pData->fSecsBetweenLightSamples ) );
	}

	sValue.Format( "%d", pData->nLightMotifIndex );
	m_ctrlMotifIndex.SetText( sValue );
	
	UpdateData( VARS_TO_CONTROLS );
	
	// copy our current settings
	m_GameData = *pData;
}

void CGlobalDlg::OnEnableTextures() {

	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_TEXTURED, m_bEnableTextures );
}

void CGlobalDlg::OnChooseTexture() {

	CSettings &Settings = CSettings::GetCurrent();
	
	UpdateData( CONTROLS_TO_VARS );	

	// use the pick asset dialog to pick assets from the master file
	CPickAsset PickAssetDlg;
	PickAssetDlg.m_nOptions = CPickAsset::PICK_ASSET_TEXTURES;
	PickAssetDlg.m_sInputMasterFile = Settings.GetMasterFileLoc();
	PickAssetDlg.m_bReadOnly = TRUE;
	PickAssetDlg.m_sInitialSelection = m_sTextureName;
	PickAssetDlg.m_sInitialSelection += ".tga";
	
	if( PickAssetDlg.DoModal() == IDOK ) {
		m_sTextureName = PickAssetDlg.GetFileTitle();
		
		UpdateData( VARS_TO_CONTROLS );

		fclib_strncpy( m_GameData.szTextureName, m_sTextureName, (FDATA_TEXNAME_LEN+1) );
		Spit_pSpitDlg->DataChanged( &m_GameData );
	}
}

void CGlobalDlg::OnKillfocusEditMotifIndex() {
	
	UpdateData( CONTROLS_TO_VARS );	

	m_GameData.nLightMotifIndex = m_ctrlMotifIndex.GetTextAsLong();
	Spit_pSpitDlg->DataChanged( &m_GameData );
}


void CGlobalDlg::OnKillfocusEditSamplesPerSec() {
	f32 fSamplesPerSec = (f32)m_ctrlSamplesPerSec.GetPos();

	m_GameData.fSecsBetweenLightSamples = 1.0f / fSamplesPerSec;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnChangeEditSamplesPerSec() {
	OnKillfocusEditSamplesPerSec();	
}

void CGlobalDlg::OnKillfocusEditScaleSampledBlue() {
	f32 fScaleBlue = (f32)m_ctrlScaleBlueSamples.GetPos();
	
	m_GameData.SampledLightScale.z = fScaleBlue * (1.0f/100.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnChangeEditScaleSampledBlue() {
	OnKillfocusEditScaleSampledBlue();
}

void CGlobalDlg::OnKillfocusEditScaleSampledGreen() {
	f32 fScaleGreen = (f32)m_ctrlScaleGreenSamples.GetPos();
	
	m_GameData.SampledLightScale.y = fScaleGreen * (1.0f/100.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnChangeEditScaleSampledGreen() {
	OnKillfocusEditScaleSampledGreen();	
}

void CGlobalDlg::OnKillfocusEditScaleSampledRed() {
	f32 fScaleRed = (f32)m_ctrlScaleRedSamples.GetPos();
	
	m_GameData.SampledLightScale.x = fScaleRed * (1.0f/100.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CGlobalDlg::OnChangeEditScaleSampledRed() {
	OnKillfocusEditScaleSampledRed();	
}

LONG CGlobalDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlScaleRedSamples.GetDlgCtrlID() ) {
		OnKillfocusEditScaleSampledRed();
	} else if( wParam == m_ctrlScaleGreenSamples.GetDlgCtrlID() ) {
		OnKillfocusEditScaleSampledGreen();
	} else if( wParam == m_ctrlScaleBlueSamples.GetDlgCtrlID() ) {
		OnKillfocusEditScaleSampledBlue();
	} else if( wParam == m_ctrlSamplesPerSec.GetDlgCtrlID() ) {
		OnKillfocusEditSamplesPerSec();
	}

	return 0;
}

void CGlobalDlg::OnJitterPos() {
	
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_JITTER_POS, m_bAddPosJitter );
}

void CGlobalDlg::OnUseBubbleForce() {

	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_BUBBLE_MOTION, m_bUseBubbleForce );
}

void CGlobalDlg::UpdateFlag( u32 &rnFlags, u32 nMask, BOOL bCurrentSetting ) {
	if( bCurrentSetting ) {
		rnFlags |= nMask;
	} else {
		rnFlags &= ~nMask;
	}
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnPlaySound() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_PLAY_SOUND, m_bPlaySound );
}

void CGlobalDlg::OnPerPixelLight() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags2, FPARTICLE_DEF_FLAG2_PER_PIXEL_LIGHT, m_bPerPixelLight );
}

void CGlobalDlg::OnAutoCalcLightIntensity() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags2, FPARTICLE_DEF_FLAG2_AUTO_CALC_LIGHT_INTENSITY, m_bAutoCalcLightIntensity );
}

void CGlobalDlg::OnCoronaOn() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags2, FPARTICLE_DEF_FLAG2_LIGHT_CORONA, m_bCoronaOn );
}

void CGlobalDlg::OnPerspectiveScaleCorona() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags2, FPARTICLE_DEF_FLAG2_WORLD_SPACE_CORONA_SCALE, m_bPerspectiveScaleCorona );
}

void CGlobalDlg::OnSpikeColor() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_SPIKE_COLOR, m_bSpikeColor );
}

void CGlobalDlg::OnSpikeAlpha() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_SPIKE_ALPHA, m_bSpikeAlpha );
}

void CGlobalDlg::OnSpikeScale() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_SPIKE_SCALE, m_bSpikeScale );
}

void CGlobalDlg::OnBnClickedChooseCoronaTexture() {
	CSettings &Settings = CSettings::GetCurrent();
	
	UpdateData( CONTROLS_TO_VARS );	

	// use the pick asset dialog to pick assets from the master file
	CPickAsset PickAssetDlg;
	PickAssetDlg.m_nOptions = CPickAsset::PICK_ASSET_TEXTURES;
	PickAssetDlg.m_sInputMasterFile = Settings.GetMasterFileLoc();
	PickAssetDlg.m_bReadOnly = TRUE;
	PickAssetDlg.m_sInitialSelection = m_sCoronaTexture;
	PickAssetDlg.m_sInitialSelection += ".tga";
	
	if( PickAssetDlg.DoModal() == IDOK ) {
		m_sCoronaTexture = PickAssetDlg.GetFileTitle();
		
		UpdateData( VARS_TO_CONTROLS );

		fclib_strncpy( m_GameData.szCoronaTextureName, m_sCoronaTexture, (FDATA_TEXNAME_LEN+1) );
		Spit_pSpitDlg->DataChanged( &m_GameData );
	}
}

void CGlobalDlg::OnBnClickedClearCoronaTexture() {
	UpdateData( CONTROLS_TO_VARS );	
	m_sCoronaTexture.Empty();
	UpdateData( VARS_TO_CONTROLS );

	fclib_strncpy( m_GameData.szCoronaTextureName, m_sCoronaTexture, (FDATA_TEXNAME_LEN+1) );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnEnKillfocusEditCoronaScale() {
	UpdateData( CONTROLS_TO_VARS );	

	FMATH_CLAMP( m_fCoronaScale, 0.1f, 20.0f );
	UpdateData( VARS_TO_CONTROLS );

	m_GameData.fCoronaScale = m_fCoronaScale;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnEnKillfocusSoundName() {
	UpdateData( CONTROLS_TO_VARS );	

	fclib_strncpy( m_GameData.szSoundFxName, m_sSoundName, (FDATA_TEXNAME_LEN+1) );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CGlobalDlg::OnBnClickedSort() {
	UpdateData( CONTROLS_TO_VARS );	

	UpdateFlag( m_GameData.nFlags, FPARTICLE_DEF_FLAGS_SORT, m_bSortParticleGroup );
}
