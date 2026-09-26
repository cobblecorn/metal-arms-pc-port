// ApeConvertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ApeConvert.h"
#include "ApeConvertDlg.h"
#include "fang.h"
#include "Convert.h"
#include "OldApeFileLoader.h"
#include "ApeFileLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _NUM_FILE_SELECTION_BYTES	16384

static char _szFileBuf[_NUM_FILE_SELECTION_BYTES];

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApeConvertDlg dialog

CApeConvertDlg::CApeConvertDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CApeConvertDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApeConvertDlg)
	m_sInputFilename = _T("");
	m_sProgressText = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CApeConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApeConvertDlg)
	DDX_Control(pDX, IDCANCEL, m_buttonExit);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ctrlProgressBar);
	DDX_Text(pDX, IDC_INPUT_NAME, m_sInputFilename);
	DDX_Text(pDX, IDC_PROGRESS_TEXT, m_sProgressText);
	DDX_Control(pDX, IDC_OUTPUT, m_listboxOutput);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CApeConvertDlg, CDialog)
	//{{AFX_MSG_MAP(CApeConvertDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHOOSE_INPUT_FILE, OnChooseInputFile)
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApeConvertDlg message handlers

BOOL CApeConvertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		pSysMenu->EnableMenuItem( 2, MF_BYPOSITION | MF_GRAYED );
		pSysMenu->EnableMenuItem( 4, MF_BYPOSITION | MF_GRAYED );
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	CString s;

	s.Format( "Ape File Converter -- %d.%d.%d => %d.%d.%d   (c) Swingin' Ape Studios, Inc.",
			  CONVERT_OLD_APE_MAJOR, CONVERT_OLD_APE_MINOR, CONVERT_OLD_APE_SUB,
			  FVERSION_APE_MAJOR, FVERSION_APE_MINOR, FVERSION_APE_SUB );
	SetWindowText( s );

	m_sProgressText = "";
	m_ctrlProgressBar.SetRange( 0, 1 );
	m_ctrlProgressBar.SetPos( 0 );

	m_asFilesToCompile.RemoveAll();

	m_listboxOutput.ResetContent();
		
	// init the fang system
	fang_Init();
	m_bFangOK = fang_Startup();
	if( !m_bFangOK ) {
		// could not install the fang engine, tell the user and exit
		this->MessageBox( "Fang Engine failed to install.", 
						  "Install Failure",
						  MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 );  
		return TRUE;
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CApeConvertDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CApeConvertDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CApeConvertDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
 
void CApeConvertDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_bFangOK ) {
		// shutdown fang
		fang_Shutdown();
		m_bFangOK = FALSE;
	}
	
	CDialog::OnCancel();
}

void CApeConvertDlg::OnOK() {
	//OnCancel();
}

BOOL CApeConvertDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CApeConvertDlg::OnChooseInputFile() {
	CString sExt;

	UpdateData( CONTROLS_TO_VARS );

	sExt = "Ape Files (*.ape)|*.ape|World Files (*.wld|*.wld||";	
	// multi file selector	
	ZeroMemory( _szFileBuf, _NUM_FILE_SELECTION_BYTES );
	strncpy( _szFileBuf, (const char *)m_sInputFilename, m_sInputFilename.GetLength() );

	CFileDialog dlg( TRUE, NULL, m_sInputFilename,
					 OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
					 (const char *)sExt, this );
	dlg.m_ofn.lpstrTitle = "Select Files To Convert";
	dlg.m_ofn.lpstrFile = _szFileBuf;
	dlg.m_ofn.nMaxFile = _NUM_FILE_SELECTION_BYTES;
	if( dlg.DoModal() == IDOK ) {
		CString sFilename = dlg.GetFileTitle();
		if( sFilename.IsEmpty() ) {
			// multiple files have been selected
			m_sInputFilename.Format( "%s\\%s", _szFileBuf, &_szFileBuf[dlg.m_ofn.nFileOffset] );
			// count the number of files selected
			m_asFilesToCompile.RemoveAll();
			char *pszFilename = &_szFileBuf[dlg.m_ofn.nFileOffset];
			CString sFullFilename;
			while( *pszFilename ) {
				sFilename = pszFilename;
				pszFilename += sFilename.GetLength() + 1;
				sFullFilename.Format( "%s\\%s", _szFileBuf, sFilename );
				m_asFilesToCompile.Add( sFullFilename );
			};
		} else {
			// only 1 file hase been selected
			m_sInputFilename = dlg.GetPathName();
			// set our string array to only have this 1 file
			m_asFilesToCompile.RemoveAll();
			m_asFilesToCompile.SetSize( 1 );
			m_asFilesToCompile[0] = m_sInputFilename;
		}
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CApeConvertDlg::OnConvert() {
	CString s;
	int nLen;

	UpdateData( CONTROLS_TO_VARS );
	m_listboxOutput.ResetContent();
	UpdateData( VARS_TO_CONTROLS );

	// make sure that an input file has been selected
	if( m_sInputFilename.IsEmpty() ) {
		s = "You must selected at least 1 file before you can convert!";
		m_listboxOutput.AddString( s );
		UpdateData( VARS_TO_CONTROLS );
		return;
	}
	
	// setup the progress bar
	int nNumFiles = m_asFilesToCompile.GetSize();
	m_ctrlProgressBar.SetRange( 0, nNumFiles );
	m_ctrlProgressBar.SetPos( 0 );
	m_ctrlProgressBar.SetStep( 1 );

	// switch the cursor to the hourglass
	BeginWaitCursor(); 

	for( int nFileNum = 0; nFileNum < nNumFiles; nFileNum++ ) {
		
		// grab out name from the CStringArray if nFileNum > 0
		if( nFileNum > 0 ) {
			m_sInputFilename = m_asFilesToCompile[nFileNum];			
		}
		GetFilenameFromFullPath( m_sInputFilename, s );
		s += ".ape:";
		nLen = s.GetLength();
		while( nLen < 35 ) {
			s += ' ';
			nLen++;
		}
		m_sProgressText.Format( "Compiling %d of %d", nFileNum+1, nNumFiles );		
		UpdateData( VARS_TO_CONTROLS );

		ConvertApeFile( s );
				
		// update our progress bar
		m_ctrlProgressBar.StepIt();

		UpdateData( VARS_TO_CONTROLS );
	};
	m_sProgressText = "idle...";
	EndWaitCursor();
	UpdateData( VARS_TO_CONTROLS );
}

BOOL CApeConvertDlg::ConvertApeFile( CString &rOutputString ) {
	CString s;
#if 1

#if 1
	COldApeFileLoader OldApeFile;
	CConvertApeFile ConvertApeFile;

	// load the input file
	if( !OldApeFile.LoadApeFile( m_sInputFilename ) ) {
		// why couldn't we load the old ape file
		s.Format( "%s", OldApeFile.GetLastError() );
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}
	// convert to the new format
	if( !ConvertApeFile.Convert( OldApeFile, m_sInputFilename ) ) {
		s = "Could not write out new ape file";
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}
#else
	// just load the existing mesh
	CApeFileLoader ApeFile;

	if( !ApeFile.LoadApeFile( m_sInputFilename ) ) {
		// why couldn't we load the old ape file
		s.Format( "%s", ApeFile.GetLastError() );
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}
	u32 nNumMats = ApeFile.GetNumMeshMaterials();
	u32 i, j;
	ApeMaterial_t *pMat;
/*
	for( i=0; i < nNumMats; i++ ) {
		pMat = ApeFile.GetMatInfoFromMeshMatIndex( i );

		if( pMat->StarCommands.bNoColl ) {
			pMat->StarCommands.nCollMask = 0;
		} else if( pMat->StarCommands.nCollMask == 0 ) {
			pMat->StarCommands.nCollMask = APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING;
		}
		for( j=0; j < pMat->nLayersUsed; j++ ) {
			if( pMat->aLayer[j].StarCommands.bNoColl ) {
				pMat->aLayer[j].StarCommands.nCollMask = 0;
			} else if( pMat->aLayer[j].StarCommands.nCollMask == 0 ) {
				pMat->aLayer[j].StarCommands.nCollMask = APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING;
			}
		}
	}
*/
	// write out the new file
	FILE *pFileStream;
	ApeMesh_t *pMesh;
	ApeBone_t *pBone;
	ApeLight_t *pLight;
	ApeObject_t *pObj;
	ApeFog_t *pFog;
	ApeShape_t *pShape;
	ApeVisVolume_t *pVolume;
	ApeVisPortal_t *pPortal;
	ApeSegment_t *pSegment;
	ApeMaterial_t *pMaterial;
	ApeVert_t *pVert;
	ApeVertIndex_t *pVIndex;

	/////////////////////////////////
	// open the new file for writting
	pFileStream = _tfopen( m_sInputFilename, _T("wb") );
	if( !pFileStream ) {
		s = "Could not write out new ape file";
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}
	////////////////////////////
	// write out the mesh header
	pMesh = ApeFile.GetMeshInfo();
	pMesh->Header.nVersion = FVERSION_APE_VERSION;
	
	fwrite( pMesh, sizeof( ApeMesh_t ), 1, pFileStream );
	
	//////////////////////
	// write out the bones
	for( i=0; i < pMesh->nNumBoneNames; i++ ) {
		pBone = ApeFile.GetMeshBone( i );
		
		fwrite( pBone, sizeof( ApeBone_t ), 1, pFileStream );
	}
	
	///////////////////////
	// write out the lights
	for( i=0; i < pMesh->nNumLights; i++ ) {
		pLight = ApeFile.GetLightInfo( i );
		
		fwrite( pLight, sizeof( ApeLight_t ), 1, pFileStream );
	}
	
	////////////////////////
	// write out the objects
	for( i=0; i < pMesh->nNumObjects; i++ ) {
		pObj = ApeFile.GetObjectInfo( i );
		
		fwrite( pObj, sizeof( ApeObject_t ), 1, pFileStream );
		if( pObj->nBytesOfUserData > 0 ) {
			fwrite( &pObj[1], pObj->nBytesOfUserData, 1, pFileStream );
		}
	}
	
	/////////////////////
	// write out the fogs
	for( i=0; i < pMesh->nNumFogs; i++ ) {
		pFog = ApeFile.GetFogInfo( i );
		
		fwrite( pFog, sizeof( ApeFog_t ), 1, pFileStream );		
	}

	///////////////////////
	// write out the shapes
	for( i=0; i < pMesh->nNumShapes; i++ ) {
		pShape = ApeFile.GetShapeInfo( i );
		
		fwrite( pShape, sizeof( ApeShape_t ), 1, pFileStream );
		if( pShape->nBytesOfUserData > 0 ) {
			fwrite( &pShape[1], pShape->nBytesOfUserData, 1, pFileStream );
		}
	}

	////////////////////////////
	// write out the vis volumes
	for( i=0; i < pMesh->nNumVisVolumes; i++ ) {
		pVolume = ApeFile.GetVisVol( i );
		
		fwrite( pVolume, sizeof( ApeVisVolume_t ), 1, pFileStream );
	}

	////////////////////////////
	// write out the vis portals
	for( i=0; i < pMesh->nNumVisPortals; i++ ) {
		pPortal = ApeFile.GetVisPortal( i );
		
		fwrite( pPortal, sizeof( ApeVisPortal_t ), 1, pFileStream );
	}

	/////////////////////////
	// write out the segments
	for( i=0; i < pMesh->nNumSegments; i++ ) {
		pSegment = ApeFile.GetSegmentInfo( i );
		
		fwrite( pSegment, sizeof( ApeSegment_t ), 1, pFileStream );
		// write out the materials
		for( j=0; j < pSegment->nNumMaterials; j++ ) {
			pMaterial = ApeFile.GetMaterialInfo( i, j );
			
			fwrite( pMaterial, sizeof( ApeMaterial_t ), 1, pFileStream );
		}
		// write out the verts
		pVert = ApeFile.GetVertData( i );
		for( j=0; j < pSegment->nNumVerts; j++ ) {
			
			fwrite( &pVert[j], sizeof( ApeVert_t ), 1, pFileStream );
		}
		// write out the vert indices
		pVIndex = ApeFile.GetVertIndex( i, 0 );
		for( j=0; j < pSegment->nNumIndices; j++ ) {
			
			fwrite( &pVIndex[j], sizeof( ApeVertIndex_t ), 1, pFileStream );
		}
	}
	
	// close our file
	fclose( pFileStream );

#endif

#else

	// open the file for reading
	FILE *pFileStream = _tfopen( m_sInputFilename, _T("rb") );
	if( !pFileStream ) {
		s = "Could not open ape file for reading.";
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}
	// read the header in
	ApeMesh_t MeshInfo;
	u32 nItemsRead = fread( &MeshInfo, sizeof( ApeMesh_t ), 1, pFileStream );
	if( nItemsRead != 1 ) {
		fclose( pFileStream );
		s = "Could not read the ape file header.";
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}
	// close the file for reading
	fclose( pFileStream );

	// verify a valid .ape file
	if( MeshInfo.Header.nSignature != FVERSION_FILE_SIGNATURE || MeshInfo.nBytesInFile == 0 ) {
		// invalid file
		s = "Ape file is not valid.";
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}
	// verify the ape file version number
	if( MeshInfo.Header.nVersion != CONVERT_OLD_APE_VERSION ) {
		if( MeshInfo.Header.nVersion == FVERSION_APE_VERSION ) {
			// already the correct version
			s = "Ape file is already the correct version.";
			rOutputString += s;
			m_listboxOutput.AddString( rOutputString );
			return FALSE;
		} else {
			// unknown version
			s = "Ape file is an unhandled version, don't know how to convert.";
			rOutputString += s;
			m_listboxOutput.AddString( rOutputString );
			return FALSE;
		}
	}

	// open the file for writting
	pFileStream = _tfopen( m_sInputFilename, _T("r+b") );
	if( !pFileStream ) {
		s = "Could not open ape file for writing.";
		rOutputString += s;
		m_listboxOutput.AddString( rOutputString );
		return FALSE;
	}

	// move the file back to the beginning
	fseek( pFileStream, 0, SEEK_SET );

	// fix the header
	MeshInfo.Header.nVersion = FVERSION_APE_VERSION;

	// write out a new header
	fwrite( &MeshInfo, sizeof( ApeMesh_t ), 1, pFileStream );

	// close the file for writing
	fclose( pFileStream );

	// if the file was world, rename it to .wld	
	if( MeshInfo.bWorldFile ) {
		CString sNewName = m_sInputFilename;
		int nPeriodIndex = sNewName.ReverseFind( '.' );
		if( nPeriodIndex >= 0 ) {
			sNewName.Delete( nPeriodIndex, sNewName.GetLength() - nPeriodIndex );
		}
		sNewName += ".wld";
		CFile::Rename( m_sInputFilename, sNewName );
	}
#endif

	// print out that we converted ok
	s = "OK";
	rOutputString += s;
	m_listboxOutput.AddString( rOutputString );

	return TRUE;
}

// sPathname should be in the form: C:\hello\mike.txt and this function will
// fill sFilename with: mike.  Returns TRUE if sFilename was filled in, FALSE 
// if there was a problem and the filename could not be computed.
BOOL CApeConvertDlg::GetFilenameFromFullPath( CString &sPathname, CString &sFilename ) {
	
	if( sPathname.IsEmpty() ) {
		return FALSE;
	}
	int nSlashIndex, nPeriodIndex;
	nSlashIndex = sPathname.ReverseFind( '\\' );
	nSlashIndex++;// this may be zero if there was no \ found
	nPeriodIndex = sPathname.ReverseFind( '.' );
	if( nPeriodIndex < 0 ) {
		// no period found, just copy from the slash to the end
		sFilename = sPathname.Mid( nSlashIndex );
	} else {
		// there was a period, I'll assume that is the extension seperator and 
		// copy from the slash to the period
		sFilename = sPathname.Mid( nSlashIndex, nPeriodIndex-nSlashIndex );
	}
	return TRUE;
}


	
