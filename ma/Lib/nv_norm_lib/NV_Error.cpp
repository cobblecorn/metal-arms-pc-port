/*********************************************************************NVMH2****
Path:  C:\Dev\devrel\Nv_sdk_4\Dx8\CommonSrc
File:  NV_Error.cpp

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.


Comments:
//	error.cpp

//	Generic error handling routines for the goodness of all...
//	This one's really simple for now
//
//  *** String size must be less than 2048.  This is hard coded for now.
//
//	Any memory cleanup/de-allocation should be done before calling the
//		DoError function with fatal == TRUE
//		function.
//
//
******************************************************************************/


#include	"NV_Error.h"
#include    <stdlib.h>          // for exit()
#include    <stdio.h>
#include    <windows.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void DoError( char * errorString, bool fatal )
{
	
	OkMsgBox( "DoError call", errorString );	

	if( fatal )
		exit(0);	//  AfxAbort();		// This not too good, as can do more sophis.
					// exception handling, but will have to do for now.

}



void OkMsgBox( char * szCaption, char * szFormat, ... )
{
	static char szBuffer[2048];
	char *pArguments;

	pArguments = (char *) &szFormat + sizeof( szFormat );
	vsprintf( szBuffer, szFormat, pArguments );
	MessageBox( NULL, szBuffer, szCaption, MB_OK );
}


#ifdef _DEBUG
	void FDebug ( char * szFormat, ... )
	{	
		static char buffer[2048];
		char *pArgs;

		pArgs = (char*) &szFormat + sizeof( szFormat );
		vsprintf( buffer, szFormat, pArgs );

		OutputDebugString ( buffer );

		Sleep( 2 );		// can miss some if calling too fast!
	}

	void NullFunc( char * szFormat, ... ) {}

	#if 0
		#define WMDIAG(str) { OutputDebugString(str); }
	#else
		#define WMDIAG(str) {}
	#endif
#else
	void FDebug( char * szFormat, ... )		{}
	void NullFunc( char * szFormat, ... )	{}

	#define WMDIAG(str) {}
#endif


void FMsg( char * szFormat, ... )
{	
	static char buffer[2048];
	char *pArgs;

	pArgs = (char*) &szFormat + sizeof( szFormat );
	vsprintf( buffer, szFormat, pArgs );

	OutputDebugString ( buffer );

	Sleep( 2 );		// can miss some if calling too fast!
}

