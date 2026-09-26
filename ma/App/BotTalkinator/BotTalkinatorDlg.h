// BotTalkinatorDlg.h : header file
//

#if !defined(AFX_BOTTALKINATORDLG_H__F3AEE01C_5C57_4D7F_A6B4_E7F70BCFEE7D__INCLUDED_)
#define AFX_BOTTALKINATORDLG_H__F3AEE01C_5C57_4D7F_A6B4_E7F70BCFEE7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBotTalkinatorDlg dialog

#include "Dialogue.h"
#include "Fang.h"
#include "floop.h"
#include "fvid.h"
#include "fmesh.h"
#include "fanim.h"
#include "MyListCtrl.h"
#include "Settings.h"

class CBotTalkinatorDlg;

struct GameLoopParam_s {
	CBotTalkinatorDlg *pDlg;
	FVidWin_t oVidSettings;
};

#define BOTTALKINATOR_MSG_INITCOMPLETE (WM_USER + 300 + 0)

#define BOT_TALK_VOCAB_FILENAME		"\\BTVocab.txt"
#define BOT_TALK_GESTURES_FILENAME	"\\BTGestures.txt"
#define BOT_TALK_DIALOGUES_FILENAME	"\\BTDialogues.txt"

class CBotTalkinatorDlg : public CDialog
{
// Construction
public:
	void ResetPointers();
	CBotTalkinatorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CBotTalkinatorDlg)
	enum { IDD = IDD_BOTTALKINATOR_DIALOG };
	CStatic	m_staticOffset;
	CEdit	m_ctrlOffsetEdit;
	CEdit	m_ctrlLenEdit;
	CStatic	m_staticLen;
	CButton	m_buttonPickCsv;
	CStatic	m_oSoundFileTitle;
	CStatic	m_oSelBotRaceText;
	CButton	m_buttonPickSfx;
	CButton	m_buttonSave;
	CButton	m_oButtonExport;
	CEdit	m_oEditExFile;
	CStatic	m_oTextExFile;
	CButton	m_oCheckTalkLoop;
	CButton	m_oEditorBorder2;
	CStatic	m_oViewStaticFrame;
	CStatic	m_oEditorStaticMilVoice;
	CComboBox	m_oEditorComboMilVoice;
	CButton	m_oEditorCheckExtDrag;
	CButton	m_oEditorButtonSetPlayFrom;
	CButton	m_oEditorButtonPlayFrom;
	CStatic	m_oEditorStaticBank2;
	CStatic	m_oEditorStaticBank1;
	CScrollBar	m_oEditorScrollGest;
	CScrollBar	m_oEditorScrollBank;
	CMyListCtrl	m_oEditorListGesture;
	CEdit	m_oEditorEditName;
	CStatic	m_oEditorStaticName;
	CComboBox	m_oSelectorComboBotName;
	CEdit	m_oEditorEditWAVFile;
	CButton	m_oEditorCheckUseWAV;
	CButton	m_oEditorCheckIs2d;
	CButton	m_oViewPlay;
	CButton	m_oViewPause;
	CStatic	m_oEditorCurTimePos;
	CSliderCtrl	m_oEditorTimePos;
	CButton	m_oMainOptions;
	CStatic	m_oSelTitleText;
	CStatic	m_oEditorTitleText;
	CStatic	m_oEditorDialogueText;
	CButton	m_oSelBorder;
	CButton	m_oEditorBorder;
	CEdit	m_oEditorTitle;
	CEdit	m_oEditorDialogue;
	CButton	m_oMainExit;
	CButton	m_oSelSelect;
	CButton	m_oSelDelete;
	CButton	m_oSelNew;
	CButton	m_oSelNext;
	CButton	m_oSelPrev;
	CButton	m_oEditorCancel;
	CButton	m_oEditorOK;
	CComboBox	m_oTitle;
	CString	m_strVersion;
	//}}AFX_DATA

	CDialogueListN m_oDialogueList;

	CDialogueEntryN *m_pCurDE;
	CDialogueInstN m_oTempDI;			// This is a temporary DI that is used while editting.

	GameLoopParam_s m_oGameLoopData;
	BOOL m_bValidVideoMode;
	CString m_sMasterFile, m_sInputPath, m_sDialogueFile, m_sAnimFile, m_sSoundFile;

	CRITICAL_SECTION m_CriticalSection;
	
	BOOL m_bDataNeedsSaving;

	FViewport_t *m_pViewport;
	f32 m_fPlayTimePos, m_fPlayFromPos, m_fBankStart, m_fBankWidth;

	CSoundInst *m_pLastSoundPlayed;// used for milspeak sounds
	BOOL m_bWavStarted;		// used for non-milspeak sounds

	volatile BOOL m_bGameInitComplete;

	enum ViewMode_e {
		VIEWMODE_PLAY,
		VIEWMODE_PLAYFROM,
		VIEWMODE_PAUSE,
	};
	ViewMode_e m_eViewMode;
	
	enum Area_e {
		AREA_SELECTOR,
		AREA_EDITOR,
	};

	void Shutdown();

	void RefreshBotNameCombo();									// Approved.
	void RefreshTitleCombo(CDialogueEntryN *poDE);				// Approved.

	void RefreshSlider(CDialogueInstN *pDialogueInst);
	void SetCurActiveArea(Area_e eActiveArea);
	void SetAreaActive(Area_e eArea, BOOL bActive);
	void ShowPreviewData(CDialogueInstN *pDialogueInst);
	void OnUpdateBankPos();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBotTalkinatorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBotTalkinatorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonNew();
	afx_msg void OnButtonPrevious();
	afx_msg void OnButtonNext();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonCancelsel();
	afx_msg void OnButtonOkaysel();
	afx_msg void OnSelchangeComboTitle();
	afx_msg void OnButtonDelete();
	virtual void OnOK();
	afx_msg void OnButtonOptions();
	virtual void OnCancel();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonPause();
	afx_msg void OnCheckEditorUsewav();
	afx_msg void OnCheckEditorIs2d();
	afx_msg void OnSelchangeComboBotname();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonPlayfrom();
	afx_msg void OnButtonSetplayfrom();
	afx_msg void OnCheckEditorExtdrag();
	afx_msg void OnCheckTalkloop();
	afx_msg void OnButtonExport();
	afx_msg void OnButtonSave();
	afx_msg void OnPickSfx();
	afx_msg void OnKillfocusEditExfile();
	afx_msg void OnPickCsv();
	afx_msg void OnKillfocusEditLen();
	afx_msg void OnKillfocusEditOffset();
	afx_msg void OnKillfocusEditTitle();
	afx_msg void OnKillfocusEditDialogue();
	afx_msg void OnKillfocusEditEditorWavfile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static BOOL GameInit(void *pParameter);
	static void GameTerm(FLoopTermCode_t nTermCode, void *pParameter);
	static BOOL GameMain(BOOL bExitRequest, void *pParameter);

	long OnInitComplete(WPARAM wParam, LPARAM lParam);

	BOOL InitApplication();
public:
	afx_msg void OnBnClickedCheckEditorUsewav2();
	afx_msg void OnBnClickedCheckEditorIs2d();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOTTALKINATORDLG_H__F3AEE01C_5C57_4D7F_A6B4_E7F70BCFEE7D__INCLUDED_)
