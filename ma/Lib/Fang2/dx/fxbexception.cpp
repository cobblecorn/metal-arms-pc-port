//////////////////////////////////////////////////////////////////////////////////////
// fxbexception.cpp - exception handler
//
// Author: Chris MacDonald
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/22/03 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "ftext.h"
//#include "fpad.h"
//#include "faudio.h"
#include "ffile.h"
#include "fclib.h"
#include "fvid.h"
#include "fdx8.h"
#include "fviewport.h"
#include "fexception.h"

//-----------------------------------------------------------------------------
// private prototypes
long _fexception_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo );
long _fexception_Production_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo );

//-----------------------------------------------------------------------------
// private variable declarations
static LPTOP_LEVEL_EXCEPTION_FILTER _pPreviousHandler;
static int _pDefaultControlWord;


//-----------------------------------------------------------------------------
BOOL fexception_ModuleStartup( void )
{
	if ( !Fang_ConfigDefs.bExceptionHandler_StartupSystem )
	{
		return TRUE;
	}

#if( FANG_PLATFORM_XB )
	return fexception_EnableHandler();
#else
	return TRUE;
#endif
}

//-----------------------------------------------------------------------------
void fexception_ModuleShutdown( void )
{
#if( FANG_PLATFORM_XB )
	fexception_DisableHandler();
#endif
}


//-----------------------------------------------------------------------------
// Enables the on-screen exception reporting functionality. Floating point exception
// handling is enabled by default, however this function will respect prior disabling
// of floating point exceptions via fexception_DisableFloatingPointExceptions().
// The function returns TRUE if handler was installed successfully, FALSE otherwise.
BOOL fexception_EnableHandler( void )
{
#if( FANG_PLATFORM_XB )
// now using __try and __except, and not SetUnhandledExceptionFilter()

//	// enable the exception handling function
//#if( !FANG_ENABLE_DEV_FEATURES )
//	_pPreviousHandler = SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER) _fexception_Production_Handler );
//#else
//	_pPreviousHandler = SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER) _fexception_Handler );
//#endif
//
	fexception_EnableFloatingPointExceptions();
#endif
	return TRUE;
}

//-----------------------------------------------------------------------------
// Disables the on-screen exception reporting functionality.
// The function returns TRUE if handler was installed successfully, FALSE otherwise.
BOOL fexception_DisableHandler( void )
{
#if( FANG_PLATFORM_XB )
	SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER) _pPreviousHandler );
	fexception_DisableFloatingPointExceptions();
#endif
	return TRUE;
}

//-----------------------------------------------------------------------------
// Allows the exception handler to respond to floating point exceptions. (On by default.)
BOOL fexception_EnableFloatingPointExceptions( void )
{
#if( FANG_PLATFORM_XB )
	// frome the Run-Time Library Reference:

	// unsigned int _controlfp( unsigned int new, unsigned int mask );

	// The _control87 (and _controlfp) function gets and sets the floating-point control word. The floating-point control 
	// word allows the program to change the precision, rounding, and infinity modes in the floating-point 
	// math package. You can also mask or unmask floating-point exceptions using _control87. If the value 
	// for mask is equal to 0, _control87 gets the floating-point control word. If mask is nonzero, a new 
	// value for the control word is set: For any bit that is on (equal to 1) in "mask", the corresponding 
	// bit in "new" is used to update the control word. In other words, fpcntrl = ((fpcntrl & ~mask) | (new & mask)) 
	// where fpcntrl is the floating-point control word.
	// _controlfp works like _control87 except that "denormal control" operations (set with mask _MCW_DN) are ignored.

	// Regarding the _MCW_EM mask, clearing the mask sets the exception, which allows the hardware exception; 
	// setting the mask hides the exception. Note that if a _EM_UNDERFLOW or_EM_OVERFLOW occurs, no hardware exception 
	// will be thrown until the next floating point instruction is executed. To generate a hardware exception immediately 
	// after _EM_UNDERFLOW or_EM_OVERFLOW, call the FWAIT MASM instruction.

	// The code below turns on FP exceptions. To trap only particular exceptions, use only the flags 
	// that correspond to the exceptions to be trapped. Note that any handler for FP errors should call 
	// _clearfp as its first FP instruction. This function clears floating-point exceptions.

	// Get the default control word. (Passing 0 in new and 0 in mask is a special case that retrieves the current word.)
//	_pDefaultControlWord = _controlfp( 0,0 );

	_controlfp( 0, EM_OVERFLOW|EM_ZERODIVIDE|EM_INVALID );
	_clearfp();
#endif
	return TRUE;
}

//-----------------------------------------------------------------------------
// Causes the exception handler to ignore floating point exceptions.
BOOL fexception_DisableFloatingPointExceptions( void )
{
#if( FANG_PLATFORM_XB )
	_controlfp( 0xffffffff, EM_OVERFLOW|EM_ZERODIVIDE|EM_INVALID );
	_clearfp();
#endif
	return TRUE;
}


#if( FANG_PLATFORM_XB )

//-----------------------------------------------------------------------------
// copy a line of text from a file to a string buffer.
// max line length is 255 chars, pLine must point to buffer of at least this size.
BOOL _ReadLine( FFileHandle hFile, char *pLine )
{
	char szBuffer[256];
	s32 nIndex, nResult;

	if( FFILE_IS_VALID_HANDLE( hFile ) )
	{
		nResult = ffile_Read( hFile, 255, (void*) szBuffer );

		if( nResult == FFILE_ERROR_RET )
		{
			pLine[0] = 0;
			return FALSE;
		}

		for( nIndex = 0; nIndex < 255; nIndex++ )
		{
			if( szBuffer[nIndex] == 0x0d )
			{
				pLine[nIndex] = 0;
				return TRUE;
			}
			else
			{
				pLine[nIndex] = szBuffer[nIndex];
			}
		}
		// error condition
		FASSERT_NOW;
		pLine[nIndex] = 0;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// result codes returned by _FindFunctionFromAddress()
typedef enum
{
	FIND_FUNCTION_RESULT_SUCCESS = 0,
	FIND_FUNCTION_RESULT_NULL_ADDRESS = -1,
	FIND_FUNCTION_RESULT_NO_MAP_FILE = -2,
	FIND_FUNCTION_RESULT_SEARCH_FAILED = -3,
	FIND_FUNCTION_RESULT_FILE_ERROR = -4,
	FIND_FUNCTION_RESULT_UNKNOWN_ERROR = -5,
} FindFunctionError_e;

//-----------------------------------------------------------------------------
// Given an exception address, this function will attempt to find the nearest
// function from a map file.
// uAddress = address of exception
// pszClassName = pointer to string buffer which will be filled with class name, if found.
// pszFunctionName = pointer to string buffer which will be filled with function name if found.
// pszTimeStamp = pointer to string buffer which will be filled with time stamp of map file, if found
// Note: string buffers should be around 256 characters.  
// This function will handle map file line lengths up to 256 characters.
FindFunctionError_e _FindFunctionFromAddress( u32 uAddress, char *pszClassName, char *pszFunctionName, char *pszTimeStamp )
{
	FFileHandle hFile;
	s32 nFilePosition;
	s32 nAddr, nLength;
	char *pszFuncStart, *pszFuncEnd, *pszClassEnd, *pszAddressStart, *pszTimeEnd;
	s32 nResult;
	BOOL bFound = FALSE;
	char szBuffer[256];

	// initialize strings
	szBuffer[0] = 0;
	pszClassName[0] = 0;
	pszFunctionName[0] = 0;
	pszTimeStamp[0] = 0;

	// validate address
	if( uAddress == NULL )
	{
		return FIND_FUNCTION_RESULT_NULL_ADDRESS;
	}

	// try to open map file on xbox
#if FANG_TEST_BUILD
	hFile = ffile_Open( "d:\\ma_xb_t.map", FFILE_OPEN_RONLY, TRUE );
#elif FANG_RELEASE_BUILD
	hFile = ffile_Open( "d:\\ma_xb_r.map", FFILE_OPEN_RONLY, TRUE );
#elif (FANG_PRODUCTION_BUILD && FANG_ENABLE_DEV_FEATURES)
	hFile = ffile_Open( "d:\\ma_xb_q.map", FFILE_OPEN_RONLY, TRUE );
#elif FANG_PRODUCTION_BUILD
	hFile = ffile_Open( "d:\\ma_xb_p.map", FFILE_OPEN_RONLY, TRUE );
#else
	return FIND_FUNCTION_RESULT_NO_MAP_FILE;
#endif

	if( !FFILE_IS_VALID_HANDLE( hFile ) )
	{
		return FIND_FUNCTION_RESULT_NO_MAP_FILE;
	}

	// get starting position of file, should be 0
	nFilePosition = ffile_Tell( hFile );

	// read lines of file until we find Timestamp line
	do
	{
		if( !_ReadLine( hFile, szBuffer ) )
		{
			return FIND_FUNCTION_RESULT_FILE_ERROR;
		}
		nFilePosition += fclib_strlen( szBuffer );
		nFilePosition += 2;
		ffile_Seek( hFile, nFilePosition, FFILE_SEEK_SET );
	}while( !fclib_strstr( szBuffer, "Timestamp" ) );

	// extract time and date information from timestamp line
	pszTimeEnd = fclib_strchr( &szBuffer[23], ')' );
	nLength = (s32)pszTimeEnd - (s32)&szBuffer[23];
	nLength += 1; // include ")"
	FMATH_CLAMP( nLength, 1, 254 );
	fclib_strncpy( pszTimeStamp, &szBuffer[23], nLength );
	pszTimeStamp[nLength] = 0;

	// read lines of file until we skip past the section listing
	do
	{
		if( !_ReadLine( hFile, szBuffer ) )
		{
			return FIND_FUNCTION_RESULT_FILE_ERROR;
		}
		nFilePosition += fclib_strlen( szBuffer );
		nFilePosition += 2;
		ffile_Seek( hFile, nFilePosition, FFILE_SEEK_SET );
	}while( !fclib_strstr( szBuffer, "Lib:Object" ) );

	// read lines of file until we find first line of section we are interested in.
	// (This is a hack, won't find non-game functions like D3D, etc.)
	do
	{
		if( !_ReadLine( hFile, szBuffer ) )
		{
			return FIND_FUNCTION_RESULT_FILE_ERROR;
		}
		nFilePosition += fclib_strlen( szBuffer );
		nFilePosition += 2;
		ffile_Seek( hFile, nFilePosition, FFILE_SEEK_SET );
	} while( !fclib_strstr( szBuffer, "0001:" ) );

	// scan through lines of file, looking for first address which is higher
	// than the exception address.  Prior line should be function in which
	// exception occurred.
	do
	{
		// see if we have exited section we are interested in
		if( !fclib_strstr( szBuffer, "0001:" ) )
		{
			break;
		}

		// skip past question marks at start of function name
		if( szBuffer[22] == '?' )
		{
			pszFuncStart = &szBuffer[23];
		}
		else
		{
			pszFuncStart = &szBuffer[22];
		}

		// find end of function name in current line
		pszFuncEnd = fclib_strchr( &szBuffer[21], '@' );
		if( pszFuncEnd == NULL )
		{
			// no '@' at end of function name, try space instead
			pszFuncEnd = fclib_strchr( &szBuffer[21], ' ' );
		}

		// find length of function name
		nLength = (s32)pszFuncEnd - (s32)pszFuncStart;
		FMATH_CLAMP( nLength, 1, 254 );

		// copy out function name
		fclib_strncpy( pszFunctionName, pszFuncStart, nLength );
		pszFunctionName[nLength] = 0;

		// if it's not a global function, find class name in current line
		if( pszFuncEnd[1] != '@' )
		{
			// find end of class name in current line
			pszClassEnd = fclib_strchr( &pszFuncEnd[1], '@' );

			if( pszClassEnd == NULL )
			{
				// no '@' at end of class name, try space instead
				pszClassEnd = fclib_strchr( &szBuffer[21], ' ' );
			}

			// find length of class name
			nLength = (s32)pszClassEnd - (s32)&pszFuncEnd[1];
			FMATH_CLAMP( nLength, 1, 254 );

			// copy out class name
			fclib_strncpy( pszClassName, &pszFuncEnd[1], nLength );
			pszClassName[nLength] = 0;
		}

		// read next line
		if( !_ReadLine( hFile, szBuffer ) )
		{
			break;
		}
		nFilePosition += fclib_strlen( szBuffer );
		nFilePosition += 2;
		nResult = ffile_Seek( hFile, nFilePosition, FFILE_SEEK_SET );

		if( nResult == FFILE_ERROR_RET || nResult == 0 )
		{
			break;
		}

		// find address of function in new line.
		// first, skip all the leading zeros.
		pszAddressStart = &szBuffer[6];
		while( *pszAddressStart == '0' && *pszAddressStart != ' ' )
		{
			pszAddressStart++;
		}

		if( *pszAddressStart == ' ' )
		{
			// address was all zeros
			nAddr = 0;
		}
		else
		{
			// convert hex number string to number
			nAddr = (s32) strtol( pszAddressStart, NULL, 16 );
		}

		// If address of new line is greater than exception address,
		// then last recorded line is closest function to exception address.
		// Note: 0x00011000 is empirically determined "magic number" that will convert
		// actual exception address to the matching address in map file.  Not sure how to get
		// this algorithmically.
		if( nAddr > (s32) uAddress - 0x00011000 )
		{
			bFound = TRUE;
			break;
		}

	} while( nResult != FFILE_ERROR_RET && nResult != 0 );

	ffile_Close( hFile );

	if( bFound )
	{
		return FIND_FUNCTION_RESULT_SUCCESS;
	}
	else
	{
		return FIND_FUNCTION_RESULT_SEARCH_FAILED;
	}
}

//-----------------------------------------------------------------------------
// exception handling function for XBox
long _fexception_Production_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	char szXBE[MAX_PATH+1];
	fclib_strcpy( szXBE, FDX8_szXBEName );
	_clearfp();

	szXBE[0] = Fang_ConfigDefs.pszFile_GameRootPathName[0];

	if ( FDX8_uLaunchType == LDT_FROM_DASHBOARD || FDX8_uLaunchType == LDT_FROM_DEBUGGER_CMDLINE )
	{
		fang_MemZero( (void*) &FDX8_LaunchData, sizeof( LAUNCH_DATA ) );
		LD_DEMO *pLauncherData = (LD_DEMO *)FDX8_LaunchData.Data;
		fclib_strcpy( pLauncherData->szLaunchedXBE, szXBE );
		fclib_strcpy( pLauncherData->szLauncherXBE, szXBE );
		XLaunchNewImage( szXBE, &FDX8_LaunchData );
	}
	else if ( FDX8_uLaunchType == LDT_TITLE )
	{
		LD_DEMO *pLauncherData = (LD_DEMO *)FDX8_LaunchData.Data;
		if ( pLauncherData )
		{
			XLaunchNewImage( pLauncherData->szLauncherXBE, &FDX8_LaunchData );
		}
		else
		{
			XLaunchNewImage( NULL, NULL );
		}
	}
	else
	{
		XLaunchNewImage( NULL, NULL );
	}


	return EXCEPTION_CONTINUE_EXECUTION;	// Return from UnhandledExceptionFilter and continue execution from the point of the exception. 
											// Note that the filter function is free to modify the continuation state by modifying the 
											// exception information supplied through its LPEXCEPTION_POINTERS parameter. 
}



//-----------------------------------------------------------------------------
// exception handling function for XBox
long _fexception_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	char szClassName[256];
	char szFunctionName[256];
	char szTimeStamp[256];
	s32 count = 0;
	FindFunctionError_e eFindResult = FIND_FUNCTION_RESULT_UNKNOWN_ERROR;

	_clearfp();

//	MEMORY_BASIC_INFORMATION mbi;
//	u32 BaseAddress = 0;
//	if( pExceptionInfo->ExceptionRecord->ExceptionAddress )
//	{
//		VirtualQuery( pExceptionInfo->ExceptionRecord->ExceptionAddress, &mbi, sizeof( mbi ) ) ;
//		BaseAddress = (u32) mbi.AllocationBase;
//	}

	if( FVid_bBegin )
	{
		fvid_End();
	}

	fdx8_InitD3DState();
	while(1)
	{
		fvid_Swap();
		fvid_Begin();
		fviewport_SetActive( NULL );
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.03f, 1.0f, 0 );

		if( pExceptionInfo )
		{
			switch( pExceptionInfo->ExceptionRecord->ExceptionCode )
			{
				case EXCEPTION_ACCESS_VIOLATION:
				//The thread tried to read from or write to a virtual address for which it does not have the appropriate access. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Virtual Address Access Violation", count );
					break;

				case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				//The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Array Address Out of Bounds", count );
					break;

				case EXCEPTION_BREAKPOINT:
				//A breakpoint was encountered. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Breakpoint Encountered", count );
					break;

				case EXCEPTION_FLT_DENORMAL_OPERAND:
				//One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Denormal Floating Point Operand", count );
					break;

				case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				//The thread tried to divide a floating-point value by a floating-point divisor of zero. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Floating Point Divide By Zero", count );
					break;

				case EXCEPTION_FLT_INEXACT_RESULT:
				//The result of a floating-point operation cannot be represented exactly as a decimal fraction. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Floating Point Inexact Result", count );
					break;

				case EXCEPTION_FLT_INVALID_OPERATION:
				//This exception represents any floating-point exception not included in this list. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Floating Point Invalid Operation", count );
					break;

				case EXCEPTION_FLT_OVERFLOW:
				//The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Floating Point Exponent Overflow", count );
					break;

				case EXCEPTION_FLT_STACK_CHECK:
				//The stack overflowed or underflowed as the result of a floating-point operation. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Stack Overflow or Underflow", count );
					break;

				case EXCEPTION_FLT_UNDERFLOW:
				//The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Floating Point Exponent Underflow", count );
					break;

				case EXCEPTION_ILLEGAL_INSTRUCTION:
				//The thread tried to execute an invalid instruction. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Illegal Instruction", count );
					break;

				case EXCEPTION_INT_DIVIDE_BY_ZERO:
				//The thread tried to divide an integer value by an integer divisor of zero. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Integer Divide By Zero", count );
					break;

				case EXCEPTION_INT_OVERFLOW:
				//The result of an integer operation caused a carry out of the most significant bit of the result. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Integer Overflow", count );
					break;

				case EXCEPTION_INVALID_DISPOSITION:
				//An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Invalid Disposition", count );
					break;

				case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				//The thread tried to continue execution after a noncontinuable exception occurred. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Non Continuable Exception", count );
					break;

				case EXCEPTION_SINGLE_STEP:
				//A trace trap or other single-instruction mechanism signaled that one instruction has been executed. 
					ftext_Printf( 0.5f, 0.30f, "~f0~C99999999~w1~ac~s0.85Single Step", count );
					break;

				default:
					break;
			}
		}

		ftext_Printf( 0.5f, 0.24f, "~f0~C99000099~w1~ac~s1.00Program Exception Occurred" );
		ftext_Printf( 0.5f, 0.33f, "~f0~C99999999~w1~ac~s0.85At Address: 0x%.8X", pExceptionInfo->ExceptionRecord->ExceptionAddress );

		if( count == 3 )
		{
			// let screen refresh a few times before trying to find the function name.
			// That way we at least get the exception printed on-screen even if
			// _FindFunctionFromAddress() somehow fails to return.
			eFindResult = _FindFunctionFromAddress( (u32) pExceptionInfo->ExceptionRecord->ExceptionAddress, szClassName, szFunctionName, szTimeStamp );
		}

		switch( eFindResult )
		{
			case FIND_FUNCTION_RESULT_SUCCESS:
				ftext_Printf( 0.5f, 0.39f, "~f0~C30803099~w1~ac~s0.85In Function:" );
				if( szClassName[0] != 0 )
				{
					ftext_Printf( 0.5f, 0.43f, "~f0~C30803099~w1~ac~s0.85%s", szClassName );
					ftext_Printf( 0.5f, 0.46f, "~f0~C30803099~w1~ac~s0.85%s", szFunctionName );
				}
				else
				{
					ftext_Printf( 0.5f, 0.43f, "~f0~C30803099~w1~ac~s0.85%s", szFunctionName );
				}

				ftext_Printf( 0.5f, 0.55f, "~f0~C30803099~w1~ac~s0.85Map File Time Stamp:" );
				ftext_Printf( 0.5f, 0.58f, "~f0~C30803099~w1~ac~s0.85%s", szTimeStamp );
				break;

			case FIND_FUNCTION_RESULT_SEARCH_FAILED:
				ftext_Printf( 0.5f, 0.39f, "~f0~C30803099~w1~ac~s0.85No Function Name" );
				ftext_Printf( 0.5f, 0.55f, "~f0~C30803099~w1~ac~s0.85Map File Time Stamp:" );
				ftext_Printf( 0.5f, 0.58f, "~f0~C30803099~w1~ac~s0.85%s", szTimeStamp );
				break;

			case FIND_FUNCTION_RESULT_NULL_ADDRESS:
				ftext_Printf( 0.5f, 0.39f, "~f0~C30803099~w1~ac~s0.85Null Exception Address" );
				break;

			case FIND_FUNCTION_RESULT_NO_MAP_FILE:
				ftext_Printf( 0.5f, 0.39f, "~f0~C30803099~w1~ac~s0.85No Map File Found" );
				break;

			default:
				break;
		}

		count++;
		fvid_End();

		if( count > 30 )
		{
			// exit this function after a brief period to allow debugger or xbWatson to catch exception as well.                           
			break;
		}
	}


//	return EXCEPTION_EXECUTE_HANDLER;	// Return from UnhandledExceptionFilter and execute the associated exception handler. 
										// This usually results in the game console freezing. 

//	return EXCEPTION_CONTINUE_EXECUTION;	// Return from UnhandledExceptionFilter and continue execution from the point of the exception. 
											// Note that the filter function is free to modify the continuation state by modifying the 
											// exception information supplied through its LPEXCEPTION_POINTERS parameter. 

	return EXCEPTION_CONTINUE_SEARCH;		// Proceed with normal execution of UnhandledExceptionFilter. 
}
#endif	//( FANG_PLATFORM_XB )
