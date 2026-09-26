//////////////////////////////////////////////////////////////////////////////////////
// Plugin.cpp - based on the skeleton exporter max exporter
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

HINSTANCE hInstance;
int controlsInit = FALSE;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
}

__declspec( dllexport ) const TCHAR *LibDescription() { 
	return GetString(IDS_LIBDESC);
}

__declspec( dllexport ) int LibNumberClasses() {
	return 1;
}

__declspec( dllexport ) ClassDesc *LibClassDesc( int i ) {
	
	switch(i) {
	case 0: 
		return GetApeSceneExportDesc();
	default:
		return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() {
	return VERSION_3DSMAX;
}

TCHAR *GetString( int id ) {
	static TCHAR buf[256];

	if( hInstance ) {
		return LoadString( hInstance, id, buf, sizeof( buf ) ) ? buf : NULL;
	}

	return NULL;
}
