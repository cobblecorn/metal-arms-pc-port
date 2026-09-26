//////////////////////////////////////////////////////////////////////////////////////
// SceneExport.cpp - based on the skeleton exporter max exporter
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
#include "SceneExport.h"
#include "ErrorLog.h"
#include "fdata.h"
#include "fclib.h"

#define _ERROR_HEADING					"3DS MAX APE FILE EXPORTER "
#define _COMPRESS_ANIMATION_FRAMES		TRUE//FALSE//
#define _ANIMATION_COMPRESS_TOLERANCE	0.001f

#define _NUM_EXTENSIONS_SUPPORTED		4

/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class ApeExportClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic()				{ return TRUE; }
	void *			Create( BOOL loading )	{ return new ApeExporter; }
	const TCHAR *	ClassName()				{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()			{ return SCENE_EXPORT_CLASS_ID; }
	Class_ID 		ClassID()				{ return APE_EXP_CLASSID; }
	const TCHAR* 	Category()				{ return _T("");  }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()			{ return _T("ApeExporter"); }
	HINSTANCE		HInstance()				{ return hInstance; }
};

static ApeExportClassDesc ApeExportCD;

ClassDesc *GetApeSceneExportDesc() {
	return &ApeExportCD;
}

// prototypes
static BOOL CALLBACK ApeMenuPrefsDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static BOOL CALLBACK WldMenuPrefsDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static BOOL CALLBACK CustomDialogHandler( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static BOOL CALLBACK AboutDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
static DWORD WINAPI fn( LPVOID arg );


// Export an ape file - this is the entry point from max
int	ApeExporter::DoExport( const TCHAR *name, ExpInterface *ei, Interface *i, 
						   BOOL suppressPrompts/*=FALSE*/, DWORD options/*=0*/ ) {
	int nReturnVal;
	INode *pRootNode;
	BOOL bCallFangShutdown = FALSE;
	TSTR sApeExt( ".ape" ), sWldExt( ".wld" ), sMtxExt( ".mtx" ), sCamExt( ".cam" );

	//////////////////////////////
	// Set local interface pointer
	m_pInterface = i;
	pRootNode = m_pInterface->GetRootNode();

	///////////////////////////////////////////////////////
	// Make sure that file name is not longer than 15 chars
	TSTR sName, sPath, sFilename, sExtension;
	sName = name;
	SplitFilename( sName, &sPath, &sFilename, &sExtension );
	if( sFilename.Length() > (MESH_NAME_LEN-1) ) {
		// name is too long
		::MessageBox( m_pInterface->GetMAXHWnd(),
					  _T("Filename is too long, please rename using 15 characters or less."), 
					  _T("Filename Error"),
					  MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON1 );
		return 1;// no need to return 0 for an "export module error" message box
	}

	//////////////////////////////////////////////////////////////////
	// Setup our error reporting system
	// get the max root directory, this is where our error log will go
	TSTR sErrorFilename;
	sErrorFilename += m_pInterface->GetDir( APP_MAXROOT_DIR );
	sErrorFilename += "\\ape_file_error_log.txt";
	CErrorLog::SetFilename( sErrorFilename );
	m_sErrorHeading.printf( "%s%s", _ERROR_HEADING, name );
	m_nNumErrors = 0;
	m_nNumCriticalErrors = 0;
	
	//////////////////////
	// Init the fang sytem
	fang_Init();
	if( !fang_Startup() ) {
		// couldn't init fang, it is needed
		RecordError( TRUE, "Could not init fang" );
		goto _Error;
	}
	bCallFangShutdown = TRUE;

	///////////////////////////////////////////
	// Load the configuration from disk
	// so that it can be used in our dialog box
	if( !LoadExporterConfig() ) {
		// could not load export config file
		RecordError( TRUE, "Could not load configuration file" );
		goto _Error;
	}

	// setup the export selected var
	m_bExportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	////////////////////////////////////////////////////////////////////////////
	// Init our left-to-right handed matrix that we use throughtout the exporter
	m_Left2RightCoordSysMtx.Identity();
	m_Left2RightCoordSysMtx.aa[1][1] = 0.0f;
	m_Left2RightCoordSysMtx.aa[1][2] = 1.0f;
	m_Left2RightCoordSysMtx.aa[2][1] = 1.0f;
	m_Left2RightCoordSysMtx.aa[2][2] = 0.0f;

	///////////////////////////////////////
	// Open a filestream for writing out to
	m_pFileStream = _tfopen( name, _T("w+b") );
	if( !m_pFileStream ) {
		RecordError( TRUE, "Could not open target file" );
		goto _Error;
	}

	////////////////////////////
	// Startup the progress bar.
	m_pInterface->ProgressStart( GetString( IDS_PROGRESS_MSG ), TRUE, fn, NULL );

	///////////////////////////////////////////////
	// Determine what type of file we are exporting
	sExtension.toLower();
	
	if( sExtension == sApeExt ) {
		nReturnVal = ApeFileMain( pRootNode, sFilename, suppressPrompts );
	} else if( sExtension == sWldExt ) {
		nReturnVal = WldFileMain( pRootNode, sFilename, suppressPrompts );
	} else if( sExtension == sMtxExt ) {
		nReturnVal = MtxFileMain( pRootNode, sFilename, suppressPrompts );
	} else if( sExtension == sCamExt ) {
		nReturnVal = CamFileMain( pRootNode, sFilename, suppressPrompts );
	} else {
		// unknown filetype, don't export
		nReturnVal = -1;
	}

	/////////////////////////////////////////////
	// Free any memory that we may have allocated
	if( m_panNodeHandles ) {
		fang_Free( m_panNodeHandles );
		m_panNodeHandles = NULL;
	}
	if( m_paPortals ) {
		fang_Free( m_paPortals );
		m_paPortals = NULL;
	}
	
	///////////////////////////
	// Always close the file!!!
	fclose( m_pFileStream );

	///////////////////////////////////////
	// We're done. Finish the progress bar.
	m_pInterface->ProgressEnd();

	if( nReturnVal == -1 ) {
		// the user canceled the export
		DeleteFile( name );
		goto _Error;
	}
	
	//////////////////////////////////////////////////
	// Save the current configuration back out to disk
	// for use next time the exporter is run
	if( !suppressPrompts ) {
		SaveExporterConfig();
	}

_Error:
	//////////////////
	// Close down fang
	if( bCallFangShutdown ) {
		fang_Shutdown();
	}

	///////////////////////////////////////////////////////////////////////
	// Give the user the option to view the error log, if there were errors
	if( m_nNumErrors ) {
		TSTR s;
		if( m_nNumCriticalErrors ) {
			s.printf( "There were critical errors while exporting:\n%s\nThe ape file is NOT valid.\nOpen the error log file?", name );
			DeleteFile( name );
		} else {
			s.printf( "There were some non-critical errors while exporting:\n%s\nThe ape file IS valid, but some items may have been skipped.\nOpen the error log file?", name );
		}
		if( ::MessageBox( m_pInterface->GetMAXHWnd(), s, "Ape Exporter Error", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			ShellExecute( NULL, "open", sErrorFilename, NULL, NULL, SW_SHOWNORMAL );
		}
	}

	return 1;// no need to return 0 for an "export module error" message box
}

/*===========================================================================*\
 |  Return how many extensions we support, and what they are
\*===========================================================================*/
int ApeExporter::ExtCount() {
	return _NUM_EXTENSIONS_SUPPORTED;
}

const TCHAR * ApeExporter::Ext( int n ) {

	switch( n ) {
	case 0:
		return GetString( IDS_EXT_01 );
		break;
	case 1:
		return GetString( IDS_EXT_02 );
		break;
	case 2:
		return GetString( IDS_EXT_03 );
		break;
	case 3:
		return GetString( IDS_EXT_04 );
		break;
	}
	return _T("");
}

/*===========================================================================*\
 |  Return various information about our scene exporter
\*===========================================================================*/
const TCHAR * ApeExporter::LongDesc() {
	return GetString( IDS_LONGDESC );
}

const TCHAR * ApeExporter::ShortDesc() {
	return GetString( IDS_SHORTDESC );
}

const TCHAR * ApeExporter::AuthorName() {
	return GetString( IDS_AUTHOR );
}

const TCHAR * ApeExporter::CopyrightMessage() {
	return GetString( IDS_COPYRIGHT );
}

const TCHAR * ApeExporter::OtherMessage1() { 
	return GetString( IDS_OTHER_MSG_01 ); 
}

const TCHAR * ApeExporter::OtherMessage2() { 
	return GetString( IDS_OTHER_MSG_02 ); 
}

// Version number = (version * 100)
unsigned int ApeExporter::Version() {
	return 200;
}

/*===========================================================================*\
 |  Show about box
\*===========================================================================*/
static BOOL CALLBACK AboutDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	ApeExporter *se = (ApeExporter *)GetWindowLong( hWnd,GWL_USERDATA );
	if( !se && msg != WM_INITDIALOG )
		return FALSE;

	switch( msg ) {
	case WM_INITDIALOG:	
		// Update class pointer
		se = (ApeExporter*)lParam;
		SetWindowLong( hWnd, GWL_USERDATA, lParam );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch( LOWORD( wParam ) ) {
		case IDC_OK:
			EndDialog( hWnd, 1 );
			break;
		}
		break;
	default:
		return FALSE;
	}	
	
	return TRUE;
}

void ApeExporter::ShowAbout( HWND hWnd ) {
	
	DialogBoxParam( hInstance,
					MAKEINTRESOURCE( IDD_ABOUT ),
					hWnd,
					AboutDlgProc,
					(LPARAM)this );
}

/*===========================================================================*\
 |  Determine what options we support
\*===========================================================================*/
BOOL ApeExporter::SupportsOptions( int ext, DWORD options )
{
	if( ext < _NUM_EXTENSIONS_SUPPORTED ) {
		return TRUE;
	}
	
	return FALSE;
}

/*===========================================================================*\
 |	Basic implimentation of a dialog handler
\*===========================================================================*/
static BOOL CALLBACK CustomDialogHandler(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int id = LOWORD(wParam);
	switch (msg) 
	{
		case WM_INITDIALOG:
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			break;
	}
	return FALSE;
}

/*===========================================================================*\
 |  Preferences dialog handler
\*===========================================================================*/
static BOOL CALLBACK ApeMenuPrefsDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	ApeExporter *se = (ApeExporter *)GetWindowLong( hWnd, GWL_USERDATA );
	if( !se && msg != WM_INITDIALOG ) {
		return FALSE;
	}

	switch (msg) {
	case WM_INITDIALOG:	
		// Update class pointer
		se = (ApeExporter *)lParam;
		SetWindowLong( hWnd, GWL_USERDATA, lParam );

		// Setup preferences initial state
		SetCheckBox( hWnd, IDC_EXPORT_LIGHTS,	se->m_bApeCfgExportLights );
		SetCheckBox( hWnd, IDC_EXPORT_GEO,		se->m_bApeCfgExportGeo );
		SetCheckBox( hWnd, IDC_EXPORT_SKIN,		se->m_bApeCfgExportSkin );
				
		// print the ape version number
		{
			TSTR sBuf;
			sBuf.printf( "Ape File Version # %d.%d.%d\nFang Tools Version # %d.%d.%d",
				FVERSION_APE_MAJOR, FVERSION_APE_MINOR, FVERSION_APE_SUB,
				fversion_GetToolMajorVer(), fversion_GetToolMinorVer(), fversion_GetToolSubVer() );
			SetWindowText( GetDlgItem( hWnd, IDC_VERSION_TEXT ), sBuf );
		}
 		break;
	case WM_DESTROY:
		break;
	case WM_SYSCOMMAND:
		if( LOWORD( wParam ) == SC_CLOSE ) {
			EndDialog(hWnd,1);
			return TRUE;
		}
		return FALSE;// all other commands should go to default handlers
		break;
	case WM_COMMAND:

		switch( LOWORD( wParam ) ) {

		case IDC_CANCEL:
			EndDialog( hWnd, 1 );
			break;

		case IDC_OK:
			// Retrieve preferences
			se->m_bApeCfgExportLights = GetCheckBox( hWnd, IDC_EXPORT_LIGHTS );
			se->m_bApeCfgExportGeo = GetCheckBox( hWnd, IDC_EXPORT_GEO );
			se->m_bApeCfgExportSkin = GetCheckBox( hWnd, IDC_EXPORT_SKIN );
			
			EndDialog( hWnd, 0 );
			break;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static BOOL CALLBACK WldMenuPrefsDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	ApeExporter *se = (ApeExporter *)GetWindowLong( hWnd, GWL_USERDATA );
	if( !se && msg != WM_INITDIALOG ) {
		return FALSE;
	}

	switch (msg) {
	case WM_INITDIALOG:	
		// Update class pointer
		se = (ApeExporter *)lParam;
		SetWindowLong( hWnd, GWL_USERDATA, lParam );

		// Setup preferences initial state
		SetCheckBox( hWnd, IDC_EXPORT_LIGHTS,	se->m_bWldCfgExportLights );
		SetCheckBox( hWnd, IDC_EXPORT_GEO,		se->m_bWldCfgExportGeo );
		SetCheckBox( hWnd, IDC_EXPORT_OBJECTS,	se->m_bWldCfgExportObjects );
				
		// print the ape version number
		{
			TSTR sBuf;
			sBuf.printf( "Ape File Version # %d.%d.%d\nFang Tools Version # %d.%d.%d",
				FVERSION_APE_MAJOR, FVERSION_APE_MINOR, FVERSION_APE_SUB,
				fversion_GetToolMajorVer(), fversion_GetToolMinorVer(), fversion_GetToolSubVer() );
			SetWindowText( GetDlgItem( hWnd, IDC_VERSION_TEXT ), sBuf );
		}
 		break;
	case WM_DESTROY:
		break;
	case WM_SYSCOMMAND:
		if( LOWORD( wParam ) == SC_CLOSE ) {
			EndDialog(hWnd,1);
			return TRUE;
		}
		return FALSE;// all other commands should go to default handlers
		break;
	case WM_COMMAND:

		switch( LOWORD( wParam ) ) {

		case IDC_CANCEL:
			EndDialog( hWnd, 1 );
			break;

		case IDC_OK:
			// Retrieve preferences
			se->m_bWldCfgExportLights = GetCheckBox( hWnd, IDC_EXPORT_LIGHTS );
			se->m_bWldCfgExportGeo = GetCheckBox( hWnd, IDC_EXPORT_GEO );
			se->m_bWldCfgExportObjects = GetCheckBox( hWnd, IDC_EXPORT_OBJECTS );
			
			EndDialog( hWnd, 0 );
			break;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static DWORD WINAPI fn( LPVOID arg ) {
	return 0;
}

// Export an ape mesh file
// returns 1 if file was exported ok
// returns 0 if there was an error
// returns -1 if the user canceled the export
//
// Enums every max node, ignoring off_ and all of their children.
// The scene is traversed once for each of the following:
// 1) Skin/Bones
// 2) Lights
// 3) Geometry
//
int	ApeExporter::ApeFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts/*=FALSE*/ ) 
{
	u32 i;
	int nNumChildren( 0 ), nMatCount( 0 ), nCount;
	CStr sErrorCode;

	////////////////////////////////
	// Show preferences setup dialog
	if( !suppressPrompts ) {
		int res = DialogBoxParam( hInstance,
								  MAKEINTRESOURCE( IDD_APE_EXP ),
								  m_pInterface->GetMAXHWnd(),
								  ApeMenuPrefsDlgProc,
								  (LPARAM)this );
		// User clicked 'Cancel'
		if( res != 0 ) {
			return -1;
		}
	}

	///////////////////////////////
	// Setup the mesh user settings
	m_bWorldFile = FALSE;
	m_bObjectFile = TRUE;
	m_bExportLights = m_bApeCfgExportLights;
	m_bExportGeo = m_bApeCfgExportGeo;
	m_bExportSkin = m_bApeCfgExportSkin;
	m_bExportObjects = FALSE;
	
	///////////////////////////////////////////////////////////////////////////////////
	// init the mesh header and write it out to our file, we will over write this later
	ZeroMemory( &m_MeshInfo, sizeof( ApeMesh_t ) );
	m_MeshInfo.Header.nSignature = FVERSION_FILE_SIGNATURE;
	m_MeshInfo.Header.nVersion = FVERSION_APE_VERSION;
	m_MeshInfo.bWorldFile = FALSE;
	m_MeshInfo.nSizeOfBoneStruct = sizeof( ApeBone_t );
	m_MeshInfo.nSizeOfLightStruct = sizeof( ApeLight_t );
	m_MeshInfo.nSizeOfObjectStruct = sizeof( ApeObject_t );
	m_MeshInfo.nSizeOfSegmentStruct = sizeof( ApeSegment_t );
	m_MeshInfo.nSizeOfMaterialStruct = sizeof( ApeMaterial_t );
	m_MeshInfo.nSizeOfVertStruct = sizeof( ApeVert_t );
	m_MeshInfo.nSizeOfVertIndexStruct = sizeof( ApeVertIndex_t );
	m_MeshInfo.nSizeOfFogStruct = sizeof( ApeFog_t );
	m_MeshInfo.nSizeOfShapeStruct = sizeof( ApeShape_t );
	strncpy( m_MeshInfo.szMeshName, rFilename, MESH_NAME_LEN-1 );
	WriteToFile( &m_MeshInfo, sizeof( ApeMesh_t ), 1, TRUE );

	// count our materials
	CountMaterials( pRootNode, nMatCount );
	m_fPercentSoFar = 0.0f;
	m_fPercentPerMat = (nMatCount) ? 100.0f/nMatCount : 100.0f;
	m_nLastPercentUpdate = -1;
	UpdateProgressBar( 1 );

	// Do we have multiple LODs?
	m_bLODsExist = FALSE;
	for ( i = 0; i < FDATA_MAX_LOD_MESH_COUNT; i++ )
	{
		apLODRootNodes[i] = NULL;
		amtxLODOffset[i].Identity();
	}
	memset( apLODRootNodes, 0, sizeof(INode *) * FDATA_MAX_LOD_MESH_COUNT );
	BOOL bOneNonLODExists = FALSE;
	nNumChildren = pRootNode->NumberOfChildren();
	for( nCount=0; nCount < nNumChildren; nCount++ ) 
	{
		INode *pChild = pRootNode->GetChildNode( nCount );
		FASSERT( pChild );

		if ( strlen( pChild->GetName() ) > 5 && strncmp( pChild->GetName(), "LOD", 3 ) == 0 && pChild->GetName()[4] == '_' )
		{
			u32 nLODIndex = pChild->GetName()[3] - '0';
			if ( nLODIndex == FDATA_MAX_LOD_MESH_COUNT )
			{
				sErrorCode.printf( "Exporter supports only up to %d LOD meshes.", FDATA_MAX_LOD_MESH_COUNT );
				RecordError( TRUE, sErrorCode, pChild );
				return FALSE;
			}
			if ( apLODRootNodes[nLODIndex] != NULL )
			{
				sErrorCode.printf( "Multiple LOD meshes in scene for LOD %d.", nLODIndex );
				RecordError( TRUE, sErrorCode, pChild );
				return FALSE;
			}
			apLODRootNodes[nLODIndex] = pChild;
			amtxLODOffset[nLODIndex] = (CFMtx43 &)pChild->GetNodeTM(0);
			amtxLODOffset[nLODIndex].Invert();
			if ( bOneNonLODExists )
			{
				RecordError( TRUE, "Exporter does not support LOD meshes and non-LOD meshes in the same scene.", pChild );
				return FALSE;
			}
			m_bLODsExist = TRUE;
		}
		else
		{
			if ( m_bLODsExist )
			{
				RecordError( TRUE, "Exporter does not support LOD meshes and non-LOD meshes in the same scene.", pChild );
				return FALSE;
			}
			bOneNonLODExists = TRUE;
		}
	}

	if ( m_bLODsExist )
	{
		// If we have LOD's, we have to have an LOD at index 0
		if ( !apLODRootNodes[0] )
		{
			RecordError( TRUE, "To support exporting of LODs, you must have an LOD 0." );
			return FALSE;
		}
	}

	/////////////////////////
	// Export the bones first
	if( m_bExportSkin ) {
		ExportBones();
		UpdateProgressBar( 3 );
	}

	/////////////////////////
	// Export the lights next
	if( m_bExportLights ) {
		// grab the light multiplier
		m_fLightMultiplier = m_pInterface->GetLightLevel( 0, FOREVER );

		if ( m_bLODsExist )
		{
			// If we have LOD's, then we just have to export the base LOD lights
			LightNodeEnum( apLODRootNodes[0] );
		}
		else
		{
			// Simple root node -> children enumeration
			// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
			// It will then recurse to search their children, and then their children, etc
			nNumChildren = pRootNode->NumberOfChildren();
			for( nCount=0; nCount < nNumChildren; nCount++ ) {
				if( m_pInterface->GetCancel() ) {
					return -1;
				}
				LightNodeEnum( pRootNode->GetChildNode( nCount ) );
			}
		}
		UpdateProgressBar( 6 );
	}

	///////////////////////////
	// Export the geometry next
	if ( m_bExportGeo ) 
	{
		if ( m_bLODsExist )
		{
			// If we have LOD's, then we just have to export the base LOD - the others will follow
			GeometryNodeEnum( pRootNode, apLODRootNodes[0] );
		}
		else
		{
			// Simple root node -> children enumeration
			// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
			// It will then recurse to search their children, and then their children, etc
			nNumChildren = pRootNode->NumberOfChildren();
			for( nCount=0; nCount < nNumChildren; nCount++ ) 
			{
				if( m_pInterface->GetCancel() ) 
				{
					return -1;
				}
				GeometryNodeEnum( pRootNode, pRootNode->GetChildNode( nCount ) );
			}
		}
	}

	// test all of our 16bit values for overflow conditions
	FASSERT( m_MeshInfo.nNumBoneNames < 0xFFFF );
	FASSERT( m_MeshInfo.nNumVisVolumes < 0xFFFF );
	FASSERT( m_MeshInfo.nNumLights < 0xFFFF );
	FASSERT( m_MeshInfo.nNumVisPortals < 0xFFFF );
	FASSERT( m_MeshInfo.nNumObjects < 0xFFFF );
	FASSERT( m_MeshInfo.nNumFogs < 0xFFFF );
	FASSERT( m_MeshInfo.nNumSegments < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfBoneStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfLightStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfObjectStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfFogStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfSegmentStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfMaterialStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfVertStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfVertIndexStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nNumShapes < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfShapeStruct < 0xFFFF );

	/////////////////////////////////////////////////////
	// Back the file pointer to the beginning of the file
	// and rewrite out the meshinfo data
	fseek( m_pFileStream, 0, SEEK_SET );
	WriteToFile( &m_MeshInfo, sizeof( ApeMesh_t ), 1, FALSE );

	return 1;// we finished ok
}

// Export a wld world file
// returns 1 if file was exported ok
// returns 0 if there was an error
// returns -1 if the user canceled the export
//
// Enums every max node, ignoring off_ and all of their children.
// The scene is traversed once for each of the following:
// 1) Lights
// 2) Objects
// 3) Fog
// 4) Shapes
// 5) Vis Volume & Vis Portals
// 6) Geometry
//
int	ApeExporter::WldFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts/*=FALSE*/ ) {
	int nNumChildren( 0 ), nMatCount( 0 ), nCount;

	////////////////////////////////
	// Show preferences setup dialog
	if( !suppressPrompts ) {
		int res = DialogBoxParam( hInstance,
								  MAKEINTRESOURCE( IDD_WLD_EXP ),
								  m_pInterface->GetMAXHWnd(),
								  WldMenuPrefsDlgProc,
								  (LPARAM)this );
		// User clicked 'Cancel'
		if( res != 0 ) {
			return -1;
		}
	}

	////////////////////////////////
	// Setup the world user settings
	m_bWorldFile = TRUE;
	m_bObjectFile = FALSE;
	m_bExportLights = m_bWldCfgExportLights;
	m_bExportGeo = m_bWldCfgExportGeo;
	m_bExportSkin = FALSE;
	m_bExportObjects = m_bWldCfgExportObjects;
	
	///////////////////////////////////////////////////////////////////////////////////
	// init the mesh header and write it out to our file, we will over write this later
	ZeroMemory( &m_MeshInfo, sizeof( ApeMesh_t ) );
	m_MeshInfo.Header.nSignature = FVERSION_FILE_SIGNATURE;
	m_MeshInfo.Header.nVersion = FVERSION_APE_VERSION;
	m_MeshInfo.bWorldFile = TRUE;
	m_MeshInfo.nSizeOfBoneStruct = sizeof( ApeBone_t );
	m_MeshInfo.nSizeOfLightStruct = sizeof( ApeLight_t );
	m_MeshInfo.nSizeOfObjectStruct = sizeof( ApeObject_t );
	m_MeshInfo.nSizeOfSegmentStruct = sizeof( ApeSegment_t );
	m_MeshInfo.nSizeOfMaterialStruct = sizeof( ApeMaterial_t );
	m_MeshInfo.nSizeOfVertStruct = sizeof( ApeVert_t );
	m_MeshInfo.nSizeOfVertIndexStruct = sizeof( ApeVertIndex_t );
	m_MeshInfo.nSizeOfFogStruct = sizeof( ApeFog_t );
	m_MeshInfo.nSizeOfShapeStruct = sizeof( ApeShape_t );
	strncpy( m_MeshInfo.szMeshName, rFilename, MESH_NAME_LEN-1 );
	WriteToFile( &m_MeshInfo, sizeof( ApeMesh_t ), 1, TRUE );

	// count our materials
	CountMaterials( pRootNode, nMatCount );
	m_fPercentSoFar = 0.0f;
	m_fPercentPerMat = (nMatCount) ? 100.0f/nMatCount : 100.0f;
	m_nLastPercentUpdate = -1;
	UpdateProgressBar( 1 );

	// We won't have any LODs in a WLD file
	u32 i;
	m_bLODsExist = FALSE;
	for ( i = 0; i < FDATA_MAX_LOD_MESH_COUNT; i++ )
	{
		apLODRootNodes[i] = NULL;
		amtxLODOffset[i].Identity();
	}
	memset( apLODRootNodes, 0, sizeof(INode *) * FDATA_MAX_LOD_MESH_COUNT );

	/////////////////////////
	// Export the lights next
	if( m_bExportLights ) {
		// grab the light multiplier
		m_fLightMultiplier = m_pInterface->GetLightLevel( 0, FOREVER );

		// Simple root node -> children enumeration
		// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
		// It will then recurse to search their children, and then their children, etc
		nNumChildren = pRootNode->NumberOfChildren();
		for( nCount=0; nCount < nNumChildren; nCount++ ) {
			if( m_pInterface->GetCancel() ) {
				return -1;
			}
			LightNodeEnum( pRootNode->GetChildNode( nCount ) );
		}
		UpdateProgressBar( 6 );
	}

	// allocate an array to hold the object and shape handles
	nCount = GetTotalNodeCount( pRootNode );
	m_panNodeHandles = (u32 *)fang_MallocAndZero( nCount * sizeof( u32 ), 4 );
	if( !m_panNodeHandles ) {
		RecordError( TRUE, "Could not allocate memory to hold object & shape heirchary data" );
		return 0;	
	}
	m_nNumHandlesAllocated = nCount;
	m_nHandlesUsed = 0;

	// grab the file location before exporting the objects
	int nFileLocOfObjects = ftell( m_pFileStream );

	//////////////////////////
	// Export the objects next
	if( m_bExportObjects ) {
		// Simple root node -> children enumeration
		// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
		// It will then recurse to search their children, and then their children, etc
		nNumChildren = pRootNode->NumberOfChildren();
		for( nCount=0; nCount < nNumChildren; nCount++ ) {
			if( m_pInterface->GetCancel() ) {
				return -1;
			}
			ObjectNodeEnum( pRootNode->GetChildNode( nCount ) );
		}
		UpdateProgressBar( 10 );
	}

	//////////////////////
	// Export the fog next

	// Simple root node -> children enumeration
	// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
	nNumChildren = pRootNode->NumberOfChildren();
	for( nCount=0; nCount < nNumChildren; nCount++ ) {
		if( m_pInterface->GetCancel() ) {
			return -1;
		}
		if( !FogNodeEnum( pRootNode->GetChildNode( nCount ) ) ) {
			// the enum function returned FALSE, stop looking for fog
			break;
		}
	}
	UpdateProgressBar( 12 );
	
	/////////////////////////
	// Export the shapes next

	// Simple root node -> children enumeration
	// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
	// It will then recurse to search their children, and then their children, etc
	nNumChildren = pRootNode->NumberOfChildren();
	for( nCount=0; nCount < nNumChildren; nCount++ ) {
		if( m_pInterface->GetCancel() ) {
			return -1;
		}
		if( !ShapeNodeEnum( pRootNode->GetChildNode( nCount ) ) ) {
			// the enum function returned FALSE, stop looking for shapes
			break;
		}
	}
	UpdateProgressBar( 15 );

	if( !FixupObjAndShapeParentIndices( nFileLocOfObjects ) ) {
		RecordError( TRUE, "Could not fixup the object & shape heirchary data" );
		return 0;	
	}

	////////////////////////////////////////////////////////////////////////////////
	// Export the vis data next (only needed if we are going to export the geometry)
	if( m_bExportGeo ) {
		// allocate an array to hold the portal data
		nCount = GetTotalNodeCount( pRootNode );
		m_paPortals = (ApeVisPortal_t *)fang_MallocAndZero( nCount * sizeof( ApeVisPortal_t ), 4 );
		if( !m_paPortals ) {
			RecordError( TRUE, "Could not allocate memory to hold the portal data" );
			return 0;	
		}
		m_nNumPortalsAllocated = nCount;
		m_nPortalsUsed = 0;

		// Simple root node -> children enumeration
		// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
		// It will then recurse to search their children, and then their children, etc
		nNumChildren = pRootNode->NumberOfChildren();
		for( nCount=0; nCount < nNumChildren; nCount++ ) {
			if( m_pInterface->GetCancel() ) {
				return -1;
			}
			PortalNodeEnum( pRootNode->GetChildNode( nCount ) );
		}
		
		// write out the portal data
		if( m_nPortalsUsed ) {
			m_MeshInfo.nNumVisPortals = m_nPortalsUsed;
			WriteToFile( m_paPortals, sizeof( ApeVisPortal_t ), m_nPortalsUsed, TRUE );			
		}
		
		// free our memory
		fang_Free( m_paPortals );
		m_paPortals = NULL;
	}

	///////////////////////////
	// Export the geometry next
	if( m_bExportGeo ) {
		// Simple root node -> children enumeration
		// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
		// It will then recurse to search their children, and then their children, etc
		nNumChildren = pRootNode->NumberOfChildren();
		for( nCount=0; nCount < nNumChildren; nCount++ ) {
			if( m_pInterface->GetCancel() ) {
				return -1;
			}
			GeometryNodeEnum( pRootNode, pRootNode->GetChildNode( nCount ) );
		}
	}

	// test all of our 16bit values for overflow conditions
	FASSERT( m_MeshInfo.nNumBoneNames < 0xFFFF );
	FASSERT( m_MeshInfo.nNumVisVolumes < 0xFFFF );
	FASSERT( m_MeshInfo.nNumLights < 0xFFFF );
	FASSERT( m_MeshInfo.nNumVisPortals < 0xFFFF );
	FASSERT( m_MeshInfo.nNumObjects < 0xFFFF );
	FASSERT( m_MeshInfo.nNumFogs < 0xFFFF );
	FASSERT( m_MeshInfo.nNumSegments < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfBoneStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfLightStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfObjectStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfFogStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfSegmentStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfMaterialStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfVertStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfVertIndexStruct < 0xFFFF );
	FASSERT( m_MeshInfo.nNumShapes < 0xFFFF );
	FASSERT( m_MeshInfo.nSizeOfShapeStruct < 0xFFFF );

	/////////////////////////////////////////////////////
	// Back the file pointer to the beginning of the file
	// and rewrite out the meshinfo data
	fseek( m_pFileStream, 0, SEEK_SET );
	WriteToFile( &m_MeshInfo, sizeof( ApeMesh_t ), 1, FALSE );

	return 1;// we finished ok
}

// Export an mtx animation file
// returns 1 if file was exported ok
// returns 0 if there was an error
// returns -1 if the user canceled the export
//
// Enums every max node, ignoring off_ and all of their children
//
int	ApeExporter::MtxFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts/*=FALSE*/ ) {
	int nNodeCount, i, nBytesAllocated, nBytesToWrite, j;
	INode *pNode;
	Interval MaxInterval;
    u32 nTicksPerFrame, nKeys, nCurrentKey, nFrameRate, nInterval, nCurMtxIndex, nNumUsedBones;
    s32 nCurTime, nStartTime, nEndTime;
    BOOL bAddEndKey = FALSE, bKeepFrame, bSkippedLastFrame;
	u8 *pnAllocatedMem = NULL;
	FData_MtxFileHeader_t *pHeader;
	FData_MtxFileBone_t *pBone, *pCurBone;
	FData_MtxFileMatrix_t *pMatrix;
	char *pszNodeName;

	////////////////////////////
	// Count the number of nodes
	nNodeCount = GetTotalNodeCount( pRootNode );
	m_fPercentSoFar = 0.0f;
	m_fPercentPerMat = (nNodeCount) ? 100.0f/nNodeCount : 100.0f;
	m_nLastPercentUpdate = -1;

	/////////////////////////////////////////////
	// Find the animation info from the interface
    MaxInterval = m_pInterface->GetAnimRange();
    nTicksPerFrame = GetTicksPerFrame();
	nFrameRate = GetFrameRate();
    nStartTime = MaxInterval.Start();
    nEndTime = MaxInterval.End();

	////////////////////////////////////////////
	// Calculate the interval and number of keys
    nInterval = (nTicksPerFrame * nFrameRate) / 30;

    nKeys = (nEndTime - nStartTime) / nInterval;

    // if the sampling frequency doesn't end directly on 
    //   on the end time, then add a partial end key
	if( ((nEndTime - nStartTime) % nInterval) != 0 ) {
        bAddEndKey = TRUE;
        nKeys += 1;
    }
    // add one more key for looping
    nKeys += 1;

	/////////////////////////////////////////
	// Allocate memory to create a file image
	nBytesAllocated = sizeof( FData_MtxFileHeader_t ) +
					  ( nNodeCount * sizeof( FData_MtxFileBone_t ) ) + 
					  ( nNodeCount * nKeys * sizeof( FData_MtxFileMatrix_t ) );
	pnAllocatedMem = new u8[nBytesAllocated];
	if( !pnAllocatedMem ) {
		RecordError( TRUE, "Could not allocate memory to hold animation data" );
		return 0;
	}

	// zero memory
	fang_MemZero( pnAllocatedMem, nBytesAllocated );

	// find out how many nodes don't have the "off_" command
	nNumUsedBones = 0;
	for( i=0; i < nNodeCount; i++ ) {
		pNode = GetNodeByIndex( pRootNode, i );
				
		if( ShouldBoneBeExported( pNode ) ) {
			nNumUsedBones++;
		}
	}
	if( !nNumUsedBones ) {
		RecordError( TRUE, "There are no bones that aren't flagged as 'off_', nothing to export" );
		return 0;
	}

	// check the bone names
	for( i=0; i < nNodeCount; i++ ) {
		pNode = GetNodeByIndex( pRootNode, i );
		pszNodeName = pNode->GetName();

		for( j = (i+1); j < nNodeCount; j++ ) {
			pNode = GetNodeByIndex( pRootNode, j );
			if( fclib_stricmp( pszNodeName, pNode->GetName() ) == 0 ) {
				RecordError( TRUE, "There is another bone with this same name, please make all bone names unique and re-export.", pNode );
				return 0;
			}
		}
	}

	/////////////////////
	// Fixup our pointers
	pHeader = (FData_MtxFileHeader_t *)pnAllocatedMem;
	pBone = (FData_MtxFileBone_t *)&pHeader[1];
	pMatrix = (FData_MtxFileMatrix_t *)&pBone[nNumUsedBones];

	UpdateProgressBar( 1 );

	//////////////////////////////
	// Export the animation frames
	pHeader->nDataType = FDATA_MTX_FILE_DATA_TYPE_MATRIX;
	for( i=0, nCurMtxIndex=0; i < nNodeCount; i++ ) {

		if( m_pInterface->GetCancel() ) {
			// the user canceled the export
			delete [] pnAllocatedMem;
			return -1;
		}
		
		pNode = GetNodeByIndex( pRootNode, i );
		FASSERT( pNode );

		pszNodeName = pNode->GetName();

		if( !ShouldBoneBeExported( pNode ) ) {
			continue;
		}

		// fill in the header
		pCurBone = &pBone[pHeader->nNumBones];
		pHeader->nNumBones++;
		FASSERT( pHeader->nNumBones <= nNumUsedBones );

		// fill in the bone
		pCurBone->nFrameArrayOffset = sizeof( FData_MtxFileHeader_t ) +
									 ( nNumUsedBones * sizeof( FData_MtxFileBone_t ) ) + 
									 ( nCurMtxIndex * sizeof( FData_MtxFileMatrix_t ) );
		strncpy( pCurBone->szName, pszNodeName, FDATA_BONENAME_LEN );

		// walk the time steps, filling in the matrices
		bSkippedLastFrame = FALSE;
		for( nCurrentKey=0, nCurTime=nStartTime; nCurTime <= nEndTime; nCurTime+=nInterval, nCurrentKey++ ) {
			GetFangAnimationMtxFromNode( pNode, nCurTime, &pMatrix[nCurMtxIndex].Mtx );

			// make sure that the node doesn't have non-uniform scale
			if( DoesMtxHaveNonUniformScale( pMatrix[nCurMtxIndex].Mtx ) ) {
				// this is not allowed
				RecordError( TRUE, "Some of the frames have a non-uniform scale, this is not allowed, please fix and re-export.", pNode );
				delete [] pnAllocatedMem;
				return 0;
			}

#if _COMPRESS_ANIMATION_FRAMES					
			if( nCurrentKey > 0 && nCurrentKey < (nKeys-1) ) {
				// see if there was any change since the last frame
				bKeepFrame = !pMatrix[nCurMtxIndex].Mtx.TestTolerance( pMatrix[nCurMtxIndex-1].Mtx, _ANIMATION_COMPRESS_TOLERANCE );
				if( !bKeepFrame ) {
					bSkippedLastFrame = TRUE;
				} else {
					if( bSkippedLastFrame ) {
						// we need to go back and save out the last saved frame again so that we don't LERP 
						// between frames where there should be no motion (drift)
						nCurrentKey--;
						nCurTime -= nInterval;
						//GetFangAnimationMtxFromNode( pNode, nCurTime, &pMatrix[nCurMtxIndex].Mtx );

						// just copy the last saved mtx (this way there is absolutely no LERP between the compressed out frames)
						pMatrix[nCurMtxIndex].Mtx = pMatrix[nCurMtxIndex-1].Mtx;
					}
					bSkippedLastFrame = FALSE;
				}
			} else {
				bKeepFrame = TRUE;
				bSkippedLastFrame = FALSE;
			}
#else
			bKeepFrame = TRUE;
#endif
			
			if( bKeepFrame ) {
				// we are keep this frame, advance our counters
				pMatrix[nCurMtxIndex].fStartingSecs = (f32)( nCurTime - nStartTime ) * (1.0f/TIME_TICKSPERSEC);
				pCurBone->nNumFrames++;
				nCurMtxIndex++;
			}			
		}
		// if the sampling rate doesn't evenly end on the last frame, add a partial key frame
		if( bAddEndKey ) {
			GetFangAnimationMtxFromNode( pNode, nEndTime, &pMatrix[nCurMtxIndex].Mtx );
			pMatrix[nCurMtxIndex].fStartingSecs = (f32)( nEndTime - nStartTime ) * (1.0f/TIME_TICKSPERSEC);
			pCurBone->nNumFrames++;
			nCurMtxIndex++;			
        }

		DEVPRINTF( "Bone %d = %s, Frames = %d, Offset = %d (bytes)\n", pHeader->nNumBones-1, pCurBone->szName, pCurBone->nNumFrames, pCurBone->nFrameArrayOffset );
		
		// update the progress meter
		UpdateProgressBar( (int)( (f32)(i+1) * m_fPercentPerMat ) );
    }

	// write the file out to disk
	nBytesToWrite = (u32)&pMatrix[nCurMtxIndex] - (u32)pnAllocatedMem;
	FASSERT( nBytesToWrite <= nBytesAllocated );
	WriteToFile( pnAllocatedMem, nBytesToWrite, 1, FALSE );
	
	// free our memory
	delete [] pnAllocatedMem;

	return 1;// we finished ok
}



// Export a cam animation file
// returns 1 if file was exported ok
// returns 0 if there was an error
// returns -1 if the user canceled the export
//
// Enums every max node, ignoring all nodes that are not cameras and don't have a cam_ name
//
int	ApeExporter::CamFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts/*=FALSE*/ ) {

	int nCount ( 0 ), nNumChildren( 0 );
	nNumChildren = pRootNode->NumberOfChildren();

	///////////////////////////////////////////////////////////////////////////////////
	// First thing to do is figure out how many cameras are slated to export.
	// If there are multiple exportable cameras, print an error message stating that
	// only one camera can be exported.
	m_nCamerasToExport = 0;
	for( nCount=0; nCount < nNumChildren; nCount++ ) {
		if( m_pInterface->GetCancel() ) {
			return -1;
		}
		CameraNodeEnum( pRootNode->GetChildNode( nCount ), APE_CAMERA_ENUM_MODE_COUNT );
	}
	if( m_nCamerasToExport == 0 ) {
		RecordError( TRUE, "There are no cameras to export!", pRootNode );
		return 0;
	}
	if( m_nCamerasToExport > 1 ) {
		RecordError( TRUE, "There are multiple camaras slated for export.  Make sure only one camera is selected, or the MAX file only has one exportable camera in it", pRootNode );
		return 0;
	}

	

	///////////////////////////////////////////////////////////////////////////////////
	// init the  header and write it out to our file, we will over write this later
	ZeroMemory( &m_CamInfo, sizeof( CamInfo_t ) );
	m_CamInfo.Header.nSignature = FVERSION_FILE_SIGNATURE;
	m_CamInfo.Header.nVersion = FVERSION_APE_VERSION;
	WriteToCamFile( &m_CamInfo, sizeof( CamInfo_t ), 1, TRUE );

	//////////////////////////
	// Export the camera
	// Simple root node -> children enumeration
	// This will get the root node, and then cycle through its children (ie, the basic scene nodes)
	// It will then recurse to search their children, and then their children, etc
	for( nCount=0; nCount < nNumChildren; nCount++ ) {
		if( m_pInterface->GetCancel() ) {
			return -1;
		}
		CameraNodeEnum( pRootNode->GetChildNode( nCount ), APE_CAMERA_ENUM_MODE_EXPORT );
	}

	if( m_CamInfo.nFrames < 2 ) {
		RecordError( TRUE, "There are not enough frames for export!", pRootNode );
		return 0;
	}

	/////////////////////////////////////////////////////
	// Back the file pointer to the beginning of the file
	// and rewrite out the caminfo data
	fseek( m_pFileStream, 0, SEEK_SET );
	WriteToCamFile( &m_CamInfo, sizeof( CamInfo_t ), 1, FALSE );
	return 1;
}