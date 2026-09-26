#if !defined(AFX_COMPILEDLG_H__9961A1CE_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
#define AFX_COMPILEDLG_H__9961A1CE_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CompileDlg.h : header file
//

#include "FileInfo.h"
#include "MasterFileCompile.h"
#include "KongToWorldFile.h"
#include "KongToVisFile.h"
#include "aidfile.h"
#include "ConvertApeFile.h"


typedef enum {
	COMPDIAG_FILE_TYPE_APE = 0,
	COMPDIAG_FILE_TYPE_WLD,
	COMPDIAG_FILE_TYPE_TGA,
	COMPDIAG_FILE_TYPE_MTX,
	COMPDIAG_FILE_TYPE_CSV,
	COMPDIAG_FILE_TYPE_FNT,
	COMPDIAG_FILE_TYPE_SMA,
	COMPDIAG_FILE_TYPE_GT,
	COMPDIAG_FILE_TYPE_WVB,
	COMPDIAG_FILE_TYPE_RAW_DATA,// this is an couple different extensions depending on which platform (ie rdg on GC, rdx on XB)
	COMPDIAG_FILE_TYPE_SFB,
	COMPDIAG_FILE_TYPE_FPR,
	COMPDIAG_FILE_TYPE_CAM,

	COMPDIAG_FILE_TYPE_GENERIC
} FileType_e;


BOOL CallConverter( const CFileInfo *pFileInfo, CCompileDlg *pDlg, u32 &rnActuallyCompiled );



/////////////////////////////////////////////////////////////////////////////
// CCompileDlg dialog

#define CDLG_UPDATE_CONTROLS				(WM_USER + 600)
#define CDLG_PROCESSING_COMPLETE			(WM_USER + 601)
#define CDLG_INCREMENT_OP_PROGRESS			(WM_USER + 602)
#define CDLG_INCREMENT_SUBOP_PROGRESS		(WM_USER + 603)
#define CDLG_SET_SUBOP_PROGRESS				(WM_USER + 604)

enum
{
	CDLG_RESULT_ASSET_NAME_TOO_LONG = 0,
	CDLG_RESULT_ASSET_TYPE_UNSUPPORTED,
	CDLG_RESULT_ASSET_LOAD_UNSUCCESSFUL,
	CDLG_RESULT_UP_TO_DATE_MASTERFILE,
	CDLG_RESULT_UP_TO_DATE_LOCALLY,
	CDLG_RESULT_UP_TO_DATE_LIBRARY,
	CDLG_RESULT_COMPILE_SUCCESSFUL,
	CDLG_RESULT_COMPILE_FAILED
};

class CCompileDlg : public CDialog
{
	// Construction
	public:
		CCompileDlg(CWnd* pParent = NULL);   // standard constructor

		CStatusBarCtrl	m_StatusBar;

		static CHARFORMAT2 m_DefaultCharFormat;
		static CRichEditCtrl m_TextStatus;
		static CProgressCtrl m_ctrlOpProgressBar;
		static CProgressCtrl m_ctrlSubOpProgressBar;
		static CString m_sOpName;
		static CString m_sSubOpName;
		static CString m_sClickAnywhere;
		static u32 m_nCurrentSubOp;

		long IncrementSubOpProgress( WPARAM wParam, LPARAM lParam );
		long IncrementOpProgress( WPARAM wParam, LPARAM lParam );
		long SetSubOpProgress( WPARAM wParam, LPARAM lParam );
		long ProcessingComplete( WPARAM wParam, LPARAM lParam );

		static void InfoString( const char *pszFormat, ... );
		static void ErrorString( const char *pszFormat, ... );
		static void WriteToLog( const char *pszFormat, ... );

		static void ResetOp( char *pszOpName, u32 nSubOpCount );
		static void ResetSubOp( char *pszSubOpName, u32 nSubOpUpdateCount );

		/////////////////////////////////////
		// must be set before calling DoModal
		CFileInfoArray m_FilesToCompile;
		CFileInfoArray m_AidFiles;// .aid files for files in the FilesToCompile list
		CString m_sMasterFilename;
		BOOL m_bRebuildAll;// if TRUE will create a brand new master file
		BOOL m_bForceCompile;// if TRUE will not look at the master file date stamp, will just compile regardless
		BOOL m_bIgnoreVIS;// Rebuilds lighting information from scratch
		BOOL m_bReusePerfectVISMatchOnly;// Reuse VIS data only if it was created from the same WLD file
		BOOL m_bUpdateMasterFileVersion;// if TRUE will update the master file's version numbers
		BOOL m_bOutputDebugPics;// used to determine if mipmaps should be exported to individual files	
		BOOL m_bHoldCompilationStats;
		BOOL m_bDownsizeTextures;
		BOOL m_bCompilingPASMGeneratedAsset; // This flag is used to indicate to the compile DLG that the asset might not be in the PASM file mapping, yet
		BOOL m_bBuildingLightMaps; // When this flag is set, PASM is in the process of compiling a WLD file and generating LMs
		BOOL m_bCheckLibraryKNGFirst; // When searching for up to date KNG files, search the library first
		BOOL m_bGenerateLMSTs; // When this flag is set, lightmap STs will be generated for the APE files that are processed
		BOOL m_bGenerateVertRad; // When this flag is set, the APE will be processed to support separate vertex radiosity color streams
		BOOL m_bGenerateMeshStrips;
		BOOL m_bDetailedLightingInfo;
		TargetPlatform_e m_nTargetPlatform;
		u32 m_nNumLeftClicks;// used when to determine when the user has clicked in the window after compliation
		BOOL m_bAbortCompile;
		u32 m_nCompileResult;

		static CMasterFileCompile m_MasterFile;

		CMapStringToPtr m_MapAidFiles;// maps filenames (mike.txt) to pFileInfo pointers in m_AidFiles

		// !!Nate
		// This will contain the number of files that were actually updated
		u32 m_uNumCompiledFiles;

	// Dialog Data
		//{{AFX_DATA(CCompileDlg)
		enum { IDD = IDD_COMPILE_DIALOG };
		CProgressCtrl	m_ctrlProgressBar;
		//}}AFX_DATA


	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CCompileDlg)
		public:
		virtual int DoModal();
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:

		// Generated message map functions
		//{{AFX_MSG(CCompileDlg)
		virtual BOOL OnInitDialog();
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnAbort();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

	private:
		CWinThread *m_pWorkThread;
		long UpdateControls( WPARAM wParam, LPARAM lParam );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPILEDLG_H__9961A1CE_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
