

#ifndef __ASSETSCANDIALOG_H
#define __ASSETSCANDIALOG_H

class CPasmDlg;

// AssetScanDialog dialog

class AssetScanDialog : public CDialog
{
	DECLARE_DYNAMIC(AssetScanDialog)

public:
	AssetScanDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~AssetScanDialog();

// Dialog Data
	enum { IDD = IDD_ASSET_SCAN_DIALOG };
	CProgressCtrl	m_ScanProgressBar;

	void SetupListBox( BOOL bOn ) { m_bSetupListBox = bOn; }
	void SetProgressPercent( f32 fUnitPercent );
	void SetPasmDlg( CPasmDlg *pPasmDlg ) { m_pPasmDlg = pPasmDlg; }

	virtual int  DoModal( void );

	long UpdateBar( WPARAM wParam, LPARAM lParam );

protected:
	virtual BOOL OnInitDialog( void );
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	static UINT WorkerThread( LPVOID pParam );
	static UINT SpinnerThread( LPVOID pParam );

	CPasmDlg *m_pPasmDlg;
	CWinThread *m_pWorkThread;
	CWinThread *m_pSpinnerThread;
	BOOL m_bSetupListBox;
	volatile u32 m_nSpinnerState;

	DECLARE_MESSAGE_MAP()
};


#endif