//////////////////////////////////////////////////////////////////////////////////////
// main.cpp - Xbox main entry function.
//
// Author: Albert Yale
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
// 03/12/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fclib.h"
#include "../viewer.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static ViewerInitParm_t _ViewerInitParms;
static cchar szMasterfile[] = "d:\\mettlearms_xb.mst";

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main( int argc, char *argv[] )
{
	fang_Init();
	Fang_ConfigDefs.nRes_HeapBytes = ( 48 * 1024 * 1024 );
	Fang_ConfigDefs.pszFile_MasterFilePathName = szMasterfile;
	Fang_ConfigDefs.nText_MaxCharsPerFrame = 2000;
	Fang_ConfigDefs.nText_MaxCharsPerPrintf = 2000;
	Fang_ConfigDefs.nVtxPool_VertexCount = 7000;

	if( !fang_Startup() )
	{
		DEVPRINTF( "Fang Engine failed to start." );
		return 0;
	}

	_ViewerInitParms.VidWin.VidDev.nFlags = FVID_DEVFLAG_HW_TNL;
	_ViewerInitParms.VidWin.VidDev.nOrdinal = 0;
	_ViewerInitParms.VidWin.VidDev.nRenderer = FVID_RENDERER_HARDWARE;
	fclib_strcpy( _ViewerInitParms.VidWin.VidDev.szName, "Xbox NV2A" );
	_ViewerInitParms.VidWin.VidMode.nColorBits = 32;
	_ViewerInitParms.VidWin.VidMode.nDepthBits = 24;
	_ViewerInitParms.VidWin.VidMode.nStencilBits = 8;
	_ViewerInitParms.VidWin.VidMode.nPixelsAcross = 640;
	_ViewerInitParms.VidWin.VidMode.nPixelsDown = 480;
	_ViewerInitParms.VidWin.VidMode.nFlags = FVID_MODEFLAG_NONE;
	_ViewerInitParms.VidWin.fUnitFSAA = 0.0f;
	_ViewerInitParms.VidWin.nSwapInterval = 1;

	viewer_Start( &_ViewerInitParms );

	return 0;

} // main

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
