//////////////////////////////////////////////////////////////////////////////////////
// wpr_datatypes.cpp - the wrapper data types - only used by the wrapper systems
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/05/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "wpr_datatypes.h"
#include "fclib.h"
#include "fresload.h"


// data used to construct each of the screens, filled in from data contained in the csv file
Wpr_DataTypes_ScreenData_t *Wpr_DataTypes_paScreenData = NULL;
wchar Wpr_DataTypes_wszTempString[WPR_DATATYPES_TEMPSTRING_LENGTH];

cwchar *WprDataTypes_pwszTitleTextColor = L"00477570";
cwchar *WprDataTypes_pwszBlueTextColor = L"00477570";
cwchar *WprDataTypes_pwszHeadingTextColor = L"00477570";
cwchar *WprDataTypes_pwszSubtitleTextColor = L"70707070";
cwchar *WprDataTypes_pwszWhiteTextColor = L"70707070";
cwchar *WprDataTypes_pwszMessageTextColor = L"70707070";
cwchar *WprDataTypes_pwszButtonTextColor = L"70707070";
cwchar *WprDataTypes_pwszGrayTextColor = L"25253780";
cwchar *WprDataTypes_pwszGrayTextColor2 = L"20203080";
cwchar *WprDataTypes_pwszInstructionTextColor = L"80282985";//"77482970";
cwchar *WprDataTypes_pwszProfileLocationColor = L"45454570";
cwchar *WprDataTypes_pwszPressStartColor = L"70757585";
cwchar *WprDataTypes_pwszTeamAColor = L"25296870";
cwchar *WprDataTypes_pwszTeamBColor = L"70192270";
cwchar *WprDataTypes_pwszHiLightedBlink = L"~B9";
cwchar *WprDataTypes_pwszBlackTextColor = L"00000099";
cwchar *WprDataTypes_pwszSolidWhiteTextColor = L"99999999";



Wpr_DataTypes_Alignment_e wpr_datatypes_GetAlignmentCode( char c ) {

	switch( c ) {

	case 'L':
	case 'l':
		return WPR_DATATYPES_ALIGN_LEFT;
		break;
		
	case 'R':
	case 'r':
		return WPR_DATATYPES_ALIGN_RIGHT;
		break;

	case 'C':
	case 'c':
		return WPR_DATATYPES_ALIGN_CENTERED;
		break;

	default:
		FASSERT_NOW;
		DEVPRINTF( "Wrappers : Unknown alignment character '%c'.\n", c );
		break;
	}

	return WPR_DATATYPES_ALIGN_LEFT;
}

#if FANG_WINGC
// PC wording for the front end's storage text: the retail strings talk about Memory Cards (GameCube)
// or Memory Units and the Xbox Dashboard; on the PC saves live in a folder. Exact matches only.
static const struct { cwchar *pwszRetail, *pwszPC; } _aPcText[] = {
	{ L"Memory Card:",
	  L"Save Location:" },
	{ L"Memory Unit:",
	  L"Save Location:" },
	{ L"Free Blocks:",
	  L"Free Space:" },
	{ L"hard disk",
	  L"save folder" },
	{ L"Memory Unit",
	  L"Save Folder" },
	{ L"Xbox Dashboard",
	  L"Manage Saves" },
	{ L"Manage Memory Card",
	  L"Manage Saves" },
	{ L"Memory management",
	  L"Manage saves" },
	{ L"Launch Xbox Dashboard",
	  L"Manage saves" },
	{ L"Memory unit is unusable.",
	  L"The save folder can't be used." },
	{ L"Memory Card is unusable.",
	  L"The save folder can't be used." },
	{ L"Not enough free blocks to save a new profile.",
	  L"Not enough disk space to save a new profile." },
	{ L"Please insert Memory Unit.",
	  L"The save folder can't be found." },
	{ L"Please insert a Memory Card.",
	  L"The save folder can't be found." },
	{ L"Memory Unit is damaged.\nPlease insert alternate memory unit.",
	  L"The save folder is damaged.\nChoose another location." },
	{ L"Memory unit is damaged.\nPlease insert alternate memory unit.",
	  L"The save folder is damaged.\nChoose another location." },
	{ L"Memory Card is damaged.\nPlease insert alternate Memory Card.",
	  L"The save folder is damaged.\nChoose another location." },
	{ L"Memory unit full!",
	  L"Disk full!" },
	{ L"hard disk full!",
	  L"Disk full!" },
	{ L"Format Memory Card",
	  L"Reset Save Data" },
	{ L"Corrupted Memory Card",
	  L"Corrupted Save Data" },
	{ L"The Memory Card in",
	  L"The save data in" },
	{ L"is corrupted and needs to be formatted.",
	  L"is corrupted and needs to be reset." },
	{ L"Do you want to format?",
	  L"Do you want to reset it?" },
	{ L"Continue without formatting",
	  L"Continue without resetting" },
	{ L"Continue without Formatting",
	  L"Continue without Resetting" },
	{ L"Format",
	  L"Reset" },
	{ L"Format Error",
	  L"Reset Error" },
	{ L"Formatting the Memory Card will\nerase all previously saved data.\n\nDo you wish to proceed with\nthe format?",
	  L"Resetting will erase all\nprofiles saved in this location.\n\nDo you wish to proceed?" },
	{ L"Memory Card Error",
	  L"Save Data Error" },
	{ L"Insert a Memory Card in\n\nto save your profile.",
	  L"Choose a save location in\n\nto save your profile." },
	{ L"The Memory Card in\n\nalready has a saved profile\nwith the same name.\n\nOverwrite existing profile?",
	  L"The save location\n\nalready has a saved profile\nwith the same name.\n\nOverwrite existing profile?" },
	{ L"The Memory Card in\n\ndoesn't have enough free files or blocks\nto save a new profile.  Existing profiles\ncan be loaded and overwritten.",
	  L"The save location\n\ndoesn't have enough free space\nto save a new profile.  Existing profiles\ncan be loaded and overwritten." },
	{ L"The Memory Card in\n\nis corrupted and needs to be formatted.\n\nDo you wish to format?",
	  L"The save data in\n\nis corrupted and needs to be reset.\n\nDo you wish to reset it?" },
	{ L"The Memory Card in\n\nis damaged and cannot be used.",
	  L"The save location\n\nis damaged and cannot be used." },
	{ L"There is no Memory Card in\n\n\nPlease insert a Memory Card\nto save your profile.",
	  L"The save location\n\n\ncan't be found. Choose another\nlocation to save your profile." },
	{ L"Wrong Device in\n\n\nPlease insert a Memory Card.",
	  L"The save location\n\n\ncan't be used." },
	{ L"Saving to the Memory Card in\n\n\nDo not touch the Memory Card\nor the POWER Button.",
	  L"Saving to\n\n\nPlease do not close the game." },
	{ L"Saving to memory unit",
	  L"Saving to the save folder" },
	{ L"Wrong Device in Memory Card Slot.",
	  L"The save location can't be used." },
	{ L"Free more blocks",
	  L"Free more space" },
	{ L"free blocks to save games.",
	  L"free space to save games." },
	{ L"Memory Card Slot A",
	  L"Save Folder" },
	{ L"Memory Card Slot B",
	  L"Save Folder" },
	{ L"is full and can be managed\nfrom the Memory Card Screen.",
	  L"is full. Free some disk space\nand try again." },
	{ L"There was an error\nsaving your profile.\n\nThe memory card in\n%ls\nis damaged and cannot be used.",
	  L"There was an error\nsaving your profile.\n\nThe save location\n%ls\nis damaged and cannot be used." },
};

cwchar *wpr_datatypes_PcText( cwchar *pwszText ) {
	if( pwszText ) {
		for( u32 i=0; i < sizeof( _aPcText ) / sizeof( _aPcText[0] ); i++ ) {
			if( !fclib_wcscmp( pwszText, _aPcText[i].pwszRetail ) ) {
				return _aPcText[i].pwszPC;
			}
		}
	}
	return pwszText;
}
#endif

Wpr_DataTypes_TextType_e wpr_datatypes_GetATextType( char c ) {

	switch( c ) {

	case 'T':
	case 't':
		return WPR_DATATYPES_TITLE;
		break;
		
	case 'S':
	case 's':
		return WPR_DATATYPES_SUBTITLE;
		break;

	case 'R':
	case 'r':
		return WPR_DATATYPES_REGULAR;
		break;

	case 'K':
	case 'k':
		return WPR_DATATYPES_KEYBOARD;
		break;

	case 'M':
	case 'm':
		return WPR_DATATYPES_MESSAGE;
		break;

	case 'H':
	case 'h':
		return WPR_DATATYPES_HEADING;
		break;

	case 'I':
	case 'i':
		return WPR_DATATYPES_INSTRUCTION;
		break;

	case 'A':
	case 'a':
		return WPR_DATATYPES_TEAM_A;
		break;

	case 'B':
	case 'b':
		return WPR_DATATYPES_TEAM_B;
		break;

	default:
		FASSERT_NOW;
		DEVPRINTF( "Wrappers : Unknown type character '%c'.\n", c );
		break;
	}

	return WPR_DATATYPES_REGULAR;
}

BOOL wpr_datatypes_InitScreenTextLayoutArray( FGameDataFileHandle_t hFile, 
											 cchar *pszTableName, 
											 Wpr_DataTypes_ScreenData_t &rScreen,
											 CFStringTable &rStringTable ) {
	FGameDataTableHandle_t hTable;
	u32 i, nIndex;
	cwchar *pwszText;
	cchar *pszChar;
	FGameData_VarType_e nDataType;

	FResFrame_t ResFrame = fres_GetFrame();

	// find the text table
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
//		Why oh why is this considered an error???? Let's allow for the displaying of no text!
//		DEVPRINTF( "wpr_datatypes_InitScreenTextLayoutArray() : Could not find the table named '%s'.\n", pszTableName );
//		goto _EXIT_WITH_ERROR;

		rScreen.nNumTextElements = 0;

		return TRUE;
	}

	// figure out how may entries there are
	rScreen.nNumTextElements = fgamedata_GetNumFields( hTable );
	if( ((rScreen.nNumTextElements % 7) != 0) ||
		rScreen.nNumTextElements == 0 ) {
		DEVPRINTF( "wpr_datatypes_InitScreenTextLayoutArray() : The table named '%s' does not have the proper number of elements.\n", pszTableName );
		goto _EXIT_WITH_ERROR;
	}
	rScreen.nNumTextElements /= 7;

	// allocate memory to hold each element
	rScreen.pText = (Wpr_DataTypes_TextLayout_t *)fres_Alloc( sizeof( Wpr_DataTypes_TextLayout_t ) * rScreen.nNumTextElements );
	if( !rScreen.pText ) {
		DEVPRINTF( "wpr_datatypes_InitScreenTextLayoutArray() : Could not allocate %d text elements, out of memory.\n", rScreen.nNumTextElements );
		goto _EXIT_WITH_ERROR;
	}

	// fill in each text element
	nIndex = 0;
	for( i=0; i < rScreen.nNumTextElements; i++ ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType );

		FASSERT( nDataType != FGAMEDATA_VAR_TYPE_STRING );
		if( nDataType == FGAMEDATA_VAR_TYPE_WIDESTRING ) {
			rScreen.pText[i].pwszText = rStringTable.AddString( WPR_PC_TEXT( pwszText ) );
		} else if( nDataType == FGAMEDATA_VAR_TYPE_FLOAT ) {
			// turn the float into a string
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%d", (u32)( *(f32 *)pwszText ) );
			rScreen.pText[i].pwszText = rStringTable.AddString( Wpr_DataTypes_wszTempString );
		}				

		pszChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + 1, nDataType );
		rScreen.pText[i].nAlignment = wpr_datatypes_GetAlignmentCode( pszChar[0] );
		
		pszChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + 2, nDataType );
		rScreen.pText[i].nType = wpr_datatypes_GetATextType( pszChar[0] );
						
		rScreen.pText[i].fUnitX = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 3, nDataType );
		rScreen.pText[i].fUnitY = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 4, nDataType );
		rScreen.pText[i].fScale = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 5, nDataType );	
		rScreen.pText[i].fTickSpaceFactor = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 6, nDataType );

		nIndex += 7;
	}

	return TRUE;
	
_EXIT_WITH_ERROR:
	fres_ReleaseFrame( ResFrame );
	rScreen.nNumTextElements = 0;
	rScreen.pText = NULL;

	return FALSE;
}

BOOL wpr_datatypes_InitScreenButtonLayoutArray( FGameDataFileHandle_t hFile, 
											   cchar *pszTableName, 
											   Wpr_DataTypes_ScreenData_t &rScreen,
											   CFStringTable &rStringTable,
											   CFTexInst *pAButton,
											   CFTexInst *pBButton,
											   CFTexInst *pCButton,
											   CFTexInst *pDButton ) {
	FGameDataTableHandle_t hTable;
	u32 i, nIndex;
	cwchar *pwszText;
	cchar *pszChar;
	FGameData_VarType_e nDataType;

	FResFrame_t ResFrame = fres_GetFrame();

	// find the button table
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
//		Why oh why is this considered an error???? Let's allow for the displaying of no buttons!
//		DEVPRINTF( "wpr_datatypes_InitScreenButtonLayoutArray() : Could not find the table named '%s'.\n", pszTableName );
//		goto _EXIT_WITH_ERROR;

		rScreen.nNumButtons = 0;

		return TRUE;
	}

	// figure out how may entries there are
	rScreen.nNumButtons = fgamedata_GetNumFields( hTable );
	if( ((rScreen.nNumButtons % 6) != 0) ||
		rScreen.nNumButtons == 0 ) {
		DEVPRINTF( "wpr_datatypes_InitScreenButtonLayoutArray() : The table named '%s' does not have the proper number of elements.\n", pszTableName );
		goto _EXIT_WITH_ERROR;
	}
	rScreen.nNumButtons /= 6;

	// allocate memory to hold each element
	rScreen.paButtons = (Wpr_DataTypes_ButtonLayout_t *)fres_Alloc( sizeof( Wpr_DataTypes_ButtonLayout_t ) * rScreen.nNumButtons );
	if( !rScreen.paButtons ) {
		DEVPRINTF( "wpr_datatypes_InitScreenButtonLayoutArray() : Could not allocate %d mesh elements, out of memory.\n", rScreen.nNumButtons );
		goto _EXIT_WITH_ERROR;
	}

	// fill in each mesh element
	nIndex = 0;
	for( i=0; i < rScreen.nNumButtons; i++ ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType );
		rScreen.paButtons[i].pwszInstructions = rStringTable.AddString( WPR_PC_TEXT( pwszText ) );
		
		pszChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + 1, nDataType );
		switch( pszChar[0] ) {

		case 'A':
		case 'a':
			rScreen.paButtons[i].pTexture = pAButton;
			break;

		case 'B':
		case 'b':
			rScreen.paButtons[i].pTexture = pBButton;
			break;

		case 'Y':
		case 'y':
			rScreen.paButtons[i].pTexture = pCButton;
			break;

		case 'X':
		case 'x':
			rScreen.paButtons[i].pTexture = pDButton;
			break;

		default:
			FASSERT_NOW;
			DEVPRINTF( "wpr_datatypes_InitScreenButtonLayoutArray() : Unknown button type character '%c'.\n", pszChar[0] );
			goto _EXIT_WITH_ERROR;
			break;
		}

		rScreen.paButtons[i].fBiPolarUnitX = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 2, nDataType );
		rScreen.paButtons[i].fBiPolarUnitY = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 3, nDataType );
		rScreen.paButtons[i].fPixelSize = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 4, nDataType );
		rScreen.paButtons[i].fFontScale = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 5, nDataType );
		
		nIndex += 6;
	}			

	return TRUE;
	
_EXIT_WITH_ERROR:
	fres_ReleaseFrame( ResFrame );
	rScreen.nNumButtons = 0;
	rScreen.paButtons = NULL;

	return FALSE;
}

CFMeshInst *wpr_datatypes_FindMeshInst( cchar *pszMeshName, u32 nNumMeshes, CFMeshInst *paMeshInsts ) {
	u32 i;

	for( i=0; i < nNumMeshes; i++ ) {
		if( fclib_stricmp( pszMeshName, paMeshInsts[i].m_pMesh->szName ) == 0 ) {
			return &paMeshInsts[i];
		}
	}

	return NULL;
}

BOOL wpr_datatypes_InitScreenMeshLayoutArray( FGameDataFileHandle_t hFile, 
											 cchar *pszTableName, 
											 Wpr_DataTypes_ScreenData_t &rScreen,
											 u32 nNumMeshes, CFMeshInst *paMeshInsts ) {
	FGameDataTableHandle_t hTable;
	u32 j, k, nIndex;
	cchar *pszMeshName;
	FGameData_VarType_e nDataType;
	Wpr_DataTypes_MeshLayout_t TempMeshLayout;

	FResFrame_t ResFrame = fres_GetFrame();

	// find the mesh table
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
//		Why oh why is this considered an error???? Let's allow for the use of no meshes!
//		DEVPRINTF( "wpr_datatypes_InitScreenMeshLayoutArray() : Could not find the table named '%s'.\n", pszTableName );
//		goto _EXIT_WITH_ERROR;

		rScreen.nNumMeshElements = 0;

		return TRUE;
	}

	// figure out how may entries there are
	rScreen.nNumMeshElements = fgamedata_GetNumFields( hTable );
	if( ((rScreen.nNumMeshElements % 5) != 0) ||
		rScreen.nNumMeshElements == 0 ) {
		DEVPRINTF( "wpr_datatypes_InitScreenMeshLayoutArray() : The table named '%s' does not have the proper number of elements.\n", pszTableName );
		goto _EXIT_WITH_ERROR;
	}
	rScreen.nNumMeshElements /= 5;

	// allocate memory to hold each element
	rScreen.pMesh = (Wpr_DataTypes_MeshLayout_t *)fres_Alloc( sizeof( Wpr_DataTypes_MeshLayout_t ) * rScreen.nNumMeshElements );
	if( !rScreen.pMesh ) {
		DEVPRINTF( "wpr_datatypes_InitScreenMeshLayoutArray() : Could not allocate %d mesh elements, out of memory.\n", rScreen.nNumMeshElements );
		goto _EXIT_WITH_ERROR;
	}

	// fill in each mesh element
	nIndex = 0;
	for( j=0; j < rScreen.nNumMeshElements; j++ ) {
		pszMeshName = (cchar *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType );
		
		rScreen.pMesh[j].pMeshInst = wpr_datatypes_FindMeshInst( pszMeshName, nNumMeshes, paMeshInsts );
		if( !rScreen.pMesh[j].pMeshInst ) {
			DEVPRINTF( "wpr_datatypes_InitScreenMeshLayoutArray() : The mesh '%s' is not in the mesh list, all meshes must be added to the mesh list before using. This mesh will not be drawn.\n", pszMeshName );
		}
		rScreen.pMesh[j].fBiPolarUnitX = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 1, nDataType );
		rScreen.pMesh[j].fBiPolarUnitY = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 2, nDataType );
		rScreen.pMesh[j].fScale = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 3, nDataType );
		rScreen.pMesh[j].fDrawZ = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 4, nDataType );
		rScreen.pMesh[j].fDrawZ *= -WPR_DATATYPES_LAYER_Z;
		rScreen.pMesh[j].fDrawZ += WPR_DATATYPES_FURTHEST_LAYER_Z;
		FMATH_CLAMPMIN( rScreen.pMesh[j].fDrawZ, 1.0f );

		nIndex += 5;
	}

	// sort the mesh elements by z distance (furthest first, nearest last)
	nIndex = rScreen.nNumMeshElements;
	for( j=0; j < nIndex-1; j++ ) {
		for( k=j+1; k < nIndex; k++ ) {
			if( rScreen.pMesh[j].fDrawZ < rScreen.pMesh[k].fDrawZ ) {
				// swap j & k
				TempMeshLayout = rScreen.pMesh[j];
				rScreen.pMesh[j] = rScreen.pMesh[k];
				rScreen.pMesh[k] = TempMeshLayout;
			}
		}
	}

	return TRUE;

_EXIT_WITH_ERROR:
	fres_ReleaseFrame( ResFrame );
	rScreen.nNumMeshElements = 0;
	rScreen.pMesh = NULL;

	return FALSE;
}

CFTexInst *wpr_datatypes_InitTextureArray( FGameDataFileHandle_t hFile, 
										  cchar *pszTableName,
										  u32 nNumExpectedTextures ) {
	FGameDataTableHandle_t hTable;
	CFTexInst *paTexInsts;
	u32 i;
	cchar *pszTexName;
    FGameData_VarType_e nDataType;
	
	FResFrame_t ResFrame = fres_GetFrame();

	paTexInsts = fnew CFTexInst[nNumExpectedTextures];
	if( !paTexInsts ) {
		DEVPRINTF( "wpr_datatypes_InitTextureArray() : Could not allocate %d tex insts, out of memory.\n", nNumExpectedTextures );
		goto _EXIT_WITH_ERROR;
	}
	// find the texture names table
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_datatypes_InitTextureArray() : Could not find the textures table named '%s'.\n", pszTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
	i = fgamedata_GetNumFields( hTable );
	if( i != nNumExpectedTextures ) {
		DEVPRINTF( "wpr_datatypes_InitTextureArray() : The textures table in '%s' didn't contain %d strings as expected.\n", pszTableName, nNumExpectedTextures );
		goto _EXIT_WITH_ERROR;
	}
	// load the textures that we will need
	for( i=0; i < nNumExpectedTextures; i++ ) {
		pszTexName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		
		paTexInsts[i].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszTexName ) );
		if( !paTexInsts[i].GetTexDef() ) {
			DEVPRINTF( "wpr_datatypes_InitTextureArray() : Could not load the texture file '%s'.\n", pszTexName );
			goto _EXIT_WITH_ERROR;
		}
	}
	
	return paTexInsts;

_EXIT_WITH_ERROR:
	fdelete_array( paTexInsts );
	fres_ReleaseFrame( ResFrame );
	
	return NULL;
}

CFMeshInst *wpr_datatypes_InitMeshArray( FGameDataFileHandle_t hFile, 
										cchar *pszTableName,
										u32 &rnNumMeshes ) {
	CFMeshInst *paMeshInsts;
	FGameDataTableHandle_t hTable;
	u32 i;
	cchar *pszMeshName;
    FGameData_VarType_e nDataType;
	FMeshInit_t MeshInit;
	FMesh_t *pMesh;
	
	FResFrame_t ResFrame = fres_GetFrame();

	//////////////////////
	// find the mesh table
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_datatypes_InitMeshArray() : Could not find the mesh table named '%s'.\n", pszTableName );
		goto _EXIT_WITH_ERROR;
	}

	// get the number of fields in the table
	rnNumMeshes = fgamedata_GetNumFields( hTable );
	if( rnNumMeshes <= 0 ) {
		DEVPRINTF( "wpr_datatypes_InitMeshArray() : The mesh table didn't contain any mesh names.\n" );
		goto _EXIT_WITH_ERROR;
	}

	// allocate a mesh array
	paMeshInsts = fnew CFMeshInst[rnNumMeshes];
	if( !paMeshInsts ) {
		DEVPRINTF( "wpr_datatypes_InitMeshArray() : Could not allocate %d mesh insts, out of memory.\n", rnNumMeshes );
		goto _EXIT_WITH_ERROR;
	}

	// load each mesh and init it
	MeshInit.nFlags = FMESHINST_FLAG_NOCOLLIDE;
	MeshInit.fCullDist = 0.0f;
	MeshInit.Mtx.Identity();

	for( i=0; i < rnNumMeshes; i++ ) {
		// grab the mesh name from the csv table
		pszMeshName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );

		// load the mesh
        pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshName );
		if( !pMesh ) {
			DEVPRINTF( "wpr_datatypes_InitMeshArray() : Could not load the mesh '%s'.\n", pszMeshName );
			goto _EXIT_WITH_ERROR;
		}
				
		// init the meshinst
		MeshInit.pMesh = pMesh;
			
		paMeshInsts[i].Init( &MeshInit ); 
	}

	return paMeshInsts;

_EXIT_WITH_ERROR:
	fdelete_array( paMeshInsts );
	rnNumMeshes = 0;
	fres_ReleaseFrame( ResFrame );
	
	return NULL;
}

void wpr_datatypes_BreakDownTimeUnits( f32 fSecs, u32 &rnHours, u32 &rnMins, u32 &rnSecs ) {
	rnSecs = (u32)fSecs;
	rnHours = (u32)(fSecs * (1.0f/3600.0f));
	rnSecs -= (rnHours * 3600);	
	rnMins = rnSecs / 60;
	rnSecs -= (rnMins * 60);	
}
