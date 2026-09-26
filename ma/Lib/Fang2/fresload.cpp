//////////////////////////////////////////////////////////////////////////////////////
// fresload.cpp - Fang resource loader module.
//
// Author: Steve Ranck     
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
// 01/25/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "ffile.h"
#include "fresload.h"
#include "fres.h"
#include "flinklist.h"
#include "fmesh.h"
#include "fworld.h"
#include "fclib.h"
#include "fdata.h"
#include "fvis.h"
#include "fheap.h"
#include "ftimer.h"
#include "gcdata.h"


#if FANG_PLATFORM_PS2 || FANG_PLATFORM_GC
	#define _SLASH			'/'
	#define _DOTSLASHSTR	"./"
#else
	#define _SLASH			'\\'
	#define _DOTSLASHSTR	".\\"
#endif

#define _MAX_HANDLERS		50



typedef struct {
	char szName[FDATA_MESHNAME_LEN+1];	// Each entry in the fixup table contains this
} _WorldFixup_t;



BOOL FResload_bLoadingWorld = FALSE;

BOOL FResLoad_bLoadedForStreaming = FALSE;

static BOOL _bModuleInitialized;
static FResLoadLogCallback_t *_pFcnLogCallback;
static FResLoadProgressCallback_t *_pFcnProgressCallback;

static FResLoadReg_t _WorldRegistration;


static cchar *_pszPath;
static u32 _nPathLength;

static const FResLoadReg_t *_apRegisteredHandlers[_MAX_HANDLERS];
static u32 _nRegisteredHandlersCount;


static BOOL _FormPathName( char **ppDestPathName, const FResLoadReg_t *pRegistration, cchar *pszResName );
static BOOL _FormPathName( char **ppDestPathName, cchar *psResType, cchar *pszResName, cchar *pszExtension );
static void _DestroyPathName( char *pPathName );
static const FResLoadReg_t *_FindRegistration( cchar *psResType );
static void *_LoadResourceFromFile( cchar *pszPathName, const FResLoadReg_t *pRegistration, cchar *pszResName, cchar *psResType );
#if FANG_USE_PORTAL
	static FVisGeoHeader_t* _LoadMeshPortionOfWorld( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType );
	static FVisData_t* _LoadWorldPortionOfWorld( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, FVisGeoHeader_t *pGeoHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType );
#else
	static FWorld_t *_FixupWorld( FData_WorldDataHeader_t *pWorldHeader, cchar *pszWorldResName );
	static FMesh_t *_LoadMeshPortionOfWorld( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType );
#endif
static BOOL _InitWorldObjects( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType );
static void *_LoadWorldResourceFromFile( cchar *pszPathName, cchar *pszResName, cchar *psResType );
static s32 _ReadFromFilePosition( FFileHandle hFile, void *pDestBuf, u32 nBytesToRead, u32 nOffset );
static void _CallLogCallback( void *pBase, cchar *psResType, cchar *pszResName, cchar *pszResPathName, FResLoadLogInfo_e nInfo );


BOOL fresload_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	_pszPath = _DOTSLASHSTR;
	_nPathLength = (u32)fclib_strlen( _pszPath );

	_nRegisteredHandlersCount = 0;
	_pFcnLogCallback = NULL;
	_pFcnProgressCallback = NULL;

	_bModuleInitialized = TRUE;
	
	FResLoad_bLoadedForStreaming = FALSE;

	fres_CopyType( _WorldRegistration.sResType, FWORLD_RESTYPE );
	_WorldRegistration.pszFileExtension = "wld";
	_WorldRegistration.nMemType = FRESLOAD_MEMTYPE_TEMP;	// Ignored
	_WorldRegistration.nAlignment = 0;						// Ignored
	_WorldRegistration.pFcnCreate = NULL;					// Ignored
	_WorldRegistration.pFcnDestroy = NULL;					// Ignored

	if( !fresload_RegisterHandler( &_WorldRegistration ) ) {
		// Registration failed...
		DEVPRINTF( "fresload_ModuleStartup(): Could not register the world resource handler.\n" );
		_bModuleInitialized = FALSE;
		return FALSE;
	}

	return TRUE;
}

void fresload_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	_nRegisteredHandlersCount = 0;
	_bModuleInitialized = FALSE;
	
	//reset the path...
	_pszPath = "";
}

FResLoadLogCallback_t *fresload_SetLogCallback( FResLoadLogCallback_t *pLogCallback ) {
	FResLoadLogCallback_t *pPrevValue;

	FASSERT( _bModuleInitialized );

	pPrevValue = _pFcnLogCallback;
	_pFcnLogCallback = pLogCallback;
	return pPrevValue;
}

FResLoadLogCallback_t *fresload_GetLogCallback( void ) {
	FASSERT( _bModuleInitialized );
	return _pFcnLogCallback;
}


FResLoadProgressCallback_t *fresload_SetProgressCallback( FResLoadProgressCallback_t *pProgressCallback ) {
	FResLoadProgressCallback_t *pPrevValue;

	FASSERT( _bModuleInitialized );

	pPrevValue = _pFcnProgressCallback;
	_pFcnProgressCallback = pProgressCallback;
	return pPrevValue;
}

FResLoadProgressCallback_t *fresload_GetProgressCallback( void ) {
	FASSERT( _bModuleInitialized );
	return _pFcnProgressCallback;
}

// The data that pszPath points to must persist past this call.
void fresload_SetPath( cchar *pszPath ) {
	
	// Note: It's valid to call *this* function before fang gets initialized...
	
	if( pszPath ) {
		_pszPath = pszPath;
	} else {
		_pszPath = _DOTSLASHSTR;
	}
	
	_nPathLength = (u32)fclib_strlen( _pszPath );
}

cchar *fresload_GetPath( void ) {
	FASSERT( _bModuleInitialized );

	return _pszPath;
}

BOOL fresload_IsRegistered( cchar *psResType ) {
	FASSERT( _bModuleInitialized );

	if( _FindRegistration( psResType ) ) {
		// This type has been registered...
		return TRUE;
	}

	// Not registered...
	return FALSE;
}

// *pRegistration must persist beyond this call.
BOOL fresload_RegisterHandler( const FResLoadReg_t *pRegistration ) {
	FASSERT( _bModuleInitialized );
	FASSERT( pRegistration->nMemType>=0 && pRegistration->nMemType<FRESLOAD_MEMTYPE_COUNT );

	if( _FindRegistration( pRegistration->sResType ) ) {
		// This type has already been registered...
		return FALSE;
	}

	FASSERT( _nRegisteredHandlersCount < _MAX_HANDLERS );

	_apRegisteredHandlers[ _nRegisteredHandlersCount++ ] = pRegistration;

	return TRUE;
}

BOOL fresload_IsNameValid( cchar *pszResName ) {
	u32 i;

	FASSERT( _bModuleInitialized );

	if( pszResName == NULL ) {
		return TRUE;
	}

	for( i=0; i<=FRES_NAMELEN; i++ ) {
		if( pszResName[i] == 0 ) {
			// Name string is ok...
			return TRUE;
		}
	}

	// Name string is too long...
	return FALSE;
}

#if FHEAP_TRACK_MEM_ALLOCATIONS
void *fresload_Load_Main( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, BOOL bForStreaming ) {

	//RAFHACK -- put progress callback here!!!
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	//
	//	fresload_Load_Main must record what the Fheap_szResourceType was  
	//	  before changing it, then restore it before returning. see bottom of this function.
	char _szOldResourceType[8];
	fclib_strncpy(_szOldResourceType, Fheap_szResourceType, 6);
	fclib_strncpy( Fheap_szResourceType, psResType, 6 );

	//must do the same with the secondary filename
	cchar* _pszOldSecondaryFileName = Fheap_pszSecondaryFileName;
	u32 nLen = (u32)fclib_strlen( pszFileName );
	u32 i;
	for ( i = nLen; i > (nLen - FHEAP_MAX_MEM_TRACKER_NAME_LEN) || i > 0; i-- )
	{
		if ( pszFileName[i] == '\\' )
		{
			i++;
			break;
		}
	}
	Fheap_pszSecondaryFileName = pszFileName + i;
	//and must also do the same with the secondary line number
	u16 _nOldLineNum = Fheap_nSecondaryLineNumber;
	Fheap_nSecondaryLineNumber = (u16)nLineNum;
#else
void *fresload_Load_Main( cchar *psResType, cchar *pszResName, BOOL bForStreaming ) {
#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	const FResLoadReg_t *pRegistration;
	void *pBase = NULL;
	char *pszPathName;

	if ( !pszResName )
	{
		return NULL;
	}

	FASSERT( _bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );
	FASSERT( fresload_IsNameValid(pszResName) );

	//RAFHACK -- Put Progress callback here
	if( _pFcnProgressCallback ) {
		char tempstring[64];
		fclib_strcpy(tempstring, "loading ");
		fclib_strcat(tempstring, pszResName);
//		_pFcnProgressCallback(tempstring);
		_pFcnProgressCallback( NULL );
	}
	
	// Keep track of whether or not this load is for streaming purposes
	FResLoad_bLoadedForStreaming = bForStreaming;
	
	if( !pszResName ) {
		// don't know what file to load
		goto _ResLoadExitOnError;
	}

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	pRegistration = _FindRegistration( psResType );
	if( pRegistration == NULL ) {
		// Resource type has not been registered...
		_CallLogCallback( NULL, psResType, pszResName, "(unknown)", FRESLOAD_LOGINFO_NOT_FOUND );
		goto _ResLoadExitOnError;
	}

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	if( !_FormPathName( &pszPathName, pRegistration, pszResName ) ) {
		// Could not form path name...
		_CallLogCallback( NULL, psResType, pszResName, "(unknown)", FRESLOAD_LOGINFO_NOT_FOUND );
		goto _ResLoadExitOnError;
	}

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	pBase = fres_FindBase( psResType, pszResName );
	if( pBase ) {
		// Resource has already been loaded...
#if FANG_PLATFORM_WIN	
		ffile_AddToLogFile( pszPathName );
#endif
		_CallLogCallback( pBase, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_ALREADY_LOADED );
		_DestroyPathName( pszPathName );			
	}
	else
	{
		if( _pFcnProgressCallback ) {
			_pFcnProgressCallback( NULL );
		}
		if( !fres_CompareTypes( psResType, FWORLD_RESTYPE ) ) {
			// Standard loader...
			if( _pFcnProgressCallback ) {
				//_pFcnProgressCallback("Loading NON-WORLD resource from file");
				_pFcnProgressCallback( NULL );
			}
			pBase = _LoadResourceFromFile( pszPathName, pRegistration, pszResName, psResType );
		} else {
			if( _pFcnProgressCallback ) {
				_pFcnProgressCallback( NULL );
				//_pFcnProgressCallback("Loading WORLD resource from file");
			}
			// This is a world resource. We need to handle it
			// a little bit differently...
			pBase = _LoadWorldResourceFromFile( pszPathName, pszResName, psResType );
		}

		if( pBase ) {
			_CallLogCallback( pBase, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_LOADED_OK );
		}

		_DestroyPathName( pszPathName );
	}

	FResLoad_bLoadedForStreaming = FALSE;

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
//		_pFcnProgressCallback("leaving FresLoad_Load_Main");
	}

#if FHEAP_TRACK_MEM_ALLOCATIONS
	fclib_strncpy(Fheap_szResourceType, _szOldResourceType, 6);
	Fheap_nSecondaryLineNumber = _nOldLineNum;
	Fheap_pszSecondaryFileName = _pszOldSecondaryFileName;
#endif // FHEAP_TRACK_MEM_ALLOCATIONS

	return pBase;

_ResLoadExitOnError:
	FResLoad_bLoadedForStreaming = FALSE;

#if FHEAP_TRACK_MEM_ALLOCATIONS
	fclib_strncpy(Fheap_szResourceType, _szOldResourceType, 6);
	Fheap_nSecondaryLineNumber = _nOldLineNum;
	Fheap_pszSecondaryFileName = _pszOldSecondaryFileName;
#endif // FHEAP_TRACK_MEM_ALLOCATIONS

	return pBase;
}

// Returns the object's size on the disk, or -1 if the object doesn't exist.
s32 fresload_GetObjectSizeOnDisk( cchar *psResType, cchar *pszResName ) {
	const FResLoadReg_t *pRegistration;
	char *pszPathName;
	FFileHandle hFile;
	s32 nFileBytes;
	BOOL bDestroyPathName;

	FASSERT( _bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );
	FASSERT( fresload_IsNameValid(pszResName) );

	bDestroyPathName = FALSE;
	nFileBytes = -1;
	hFile = -1;

	pRegistration = _FindRegistration( psResType );
	if( pRegistration == NULL ) {
		// Resource type has not been registered...
		goto _ExitGetObjectSizeOnDisk;
	}

	if( !_FormPathName( &pszPathName, pRegistration, pszResName ) ) {
		// Could not form path name...
		goto _ExitGetObjectSizeOnDisk;
	}

	bDestroyPathName = TRUE;

	// Open the file...

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}
	hFile = ffile_Open( pszPathName, FFILE_OPEN_RONLY );
	if( hFile < 0 ) {
		// Could not open the file...
		goto _ExitGetObjectSizeOnDisk;
	}
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	// Determine the size of the file...
	nFileBytes = ffile_GetFileSize( hFile );
	if( nFileBytes < 0 ) {
		// Could not determine file size...
		nFileBytes = -1;
		goto _ExitGetObjectSizeOnDisk;
	}

_ExitGetObjectSizeOnDisk:

	// Close the file...
	if( hFile >= 0 ) {
		ffile_Close( hFile );
	}

	if( bDestroyPathName ) {
		_DestroyPathName( pszPathName );
	}

	return nFileBytes;
}

// Returns TRUE if the specified object exists on disk.
BOOL fresload_ExistsOnDisk( cchar *psResType, cchar *pszResName ) {
	const FResLoadReg_t *pRegistration;
	char *pszPathName;
	FFileHandle hFile;
	BOOL bDestroyPathName, bExistsOnDisk;

	FASSERT( _bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );
	FASSERT( fresload_IsNameValid(pszResName) );

	bExistsOnDisk = FALSE;
	bDestroyPathName = FALSE;
	hFile = -1;

	pRegistration = _FindRegistration( psResType );
	if( pRegistration == NULL ) {
		// Resource type has not been registered...
		goto _ExitExistsOnDisk;
	}

	if( !_FormPathName( &pszPathName, pRegistration, pszResName ) ) {
		// Could not form path name...
		goto _ExitExistsOnDisk;
	}

	bDestroyPathName = TRUE;

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	// Open the file...
	hFile = ffile_Open( pszPathName, FFILE_OPEN_RONLY );
	if( hFile < 0 ) {
		// Could not open the file...
		goto _ExitExistsOnDisk;
	}

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	bExistsOnDisk = TRUE;

_ExitExistsOnDisk:

	// Close the file...
	if( hFile >= 0 ) {
		ffile_Close( hFile );
	}

	if( bDestroyPathName ) {
		_DestroyPathName( pszPathName );
	}

	return bExistsOnDisk;
}

// Loads the raw object from disk into the specified destination buffer.
// Returns the number of bytes read, or -1 if an error occurred.
// This function does not call the log callback function, if any.
s32 fresload_LoadRaw( cchar *psResType, cchar *pszResName, void *pDestBuffer, s32 nDestBufferBytes ) {
	const FResLoadReg_t *pRegistration;
	char *pszPathName;
	FFileHandle hFile;
	s32 nFileBytes;
	BOOL bDestroyPathName;

	FASSERT( _bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );
	FASSERT( fresload_IsNameValid(pszResName) );
	FASSERT( nDestBufferBytes >= 0 );
	FASSERT( pDestBuffer );

	bDestroyPathName = FALSE;
	nFileBytes = -1;
	hFile = -1;

	pRegistration = _FindRegistration( psResType );
	if( pRegistration == NULL ) {
		// Resource type has not been registered...
		goto _ExitLoadRaw;
	}

	if( !_FormPathName( &pszPathName, pRegistration, pszResName ) ) {
		// Could not form path name...
		goto _ExitLoadRaw;
	}

	bDestroyPathName = TRUE;

	// Open the file...
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}
	hFile = ffile_Open( pszPathName, FFILE_OPEN_RONLY );
	if( hFile < 0 ) {
		// Could not open the file...
		goto _ExitLoadRaw;
	}
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	// Determine the size of the file...
	nFileBytes = ffile_GetFileSize( hFile );
	if( nFileBytes < 0 ) {
		// Could not determine file size...
		nFileBytes = -1;
		goto _ExitLoadRaw;
	}

	if( nDestBufferBytes < nFileBytes ) {
		// Destination buffer too small...
		goto _ExitLoadRaw;
	}

	// Load data from disk...
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	if( ffile_Read( hFile, (u32)nFileBytes, pDestBuffer ) < 0 ) {
		// Trouble reading from disk...
		nFileBytes = -1;
		goto _ExitLoadRaw;
	}

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

_ExitLoadRaw:

	// Close the file...
	if( hFile >= 0 ) {
		ffile_Close( hFile );
	}

	if( bDestroyPathName ) {
		_DestroyPathName( pszPathName );
	}

	return nFileBytes;
}

// Allocates memory from the main system heap and loads the specified object into it.
// Returns a pointer to the allocated memory block which holds the data, or NULL if not successful.
// If pnSizeInBytes is not NULL, it is filled with the size of the allocated buffer.
// nByteAlignment is the byte alignment for the allocated buffer, or 0 for the default.
//
// When the buffer is no longer needed it must be freed with fang_Free().
void *fresload_LoadRawIntoSysHeap( cchar *psResType, cchar *pszResName, s32 *pnSizeInBytes, u32 nByteAlignment ) {
	s32 nBufBytes, nLoadedBytes;
	void *pBuf;

	FASSERT( _bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );
	FASSERT( fresload_IsNameValid(pszResName) );

	nBufBytes = fresload_GetObjectSizeOnDisk( psResType, pszResName );

	if( nBufBytes < 0 ) {
		// Could not get file size...

		if( pnSizeInBytes ) {
			*pnSizeInBytes = 0;
		}

		return NULL;
	}

	pBuf = fang_Malloc( (u32)nBufBytes, nByteAlignment );
	if( pBuf == NULL ) {
		// Could not allocate memory...

		if( pnSizeInBytes ) {
			*pnSizeInBytes = 0;
		}

		return NULL;
	}

	nLoadedBytes = fresload_LoadRaw( psResType, pszResName, pBuf, nBufBytes );
	if( nLoadedBytes < 0 ) {
		// Could not load object...

		fang_Free( pBuf );

		if( pnSizeInBytes ) {
			*pnSizeInBytes = 0;
		}

		return NULL;
	}

	// Success loading object...

	if( pnSizeInBytes ) {
		*pnSizeInBytes = nBufBytes;
	}

	return pBuf;
}

// Allocates memory from the res heap and loads the specified object into it.
// Returns a pointer to the allocated memory block which holds the data, or NULL if not successful.
// If pnSizeInBytes is not NULL, it is filled with the size of the allocated buffer.
// nByteAlignment is the byte alignment for the allocated buffer, or 0 for the default.
void *fresload_LoadRawIntoResMem( cchar *psResType, cchar *pszResName, s32 *pnSizeInBytes, u32 nByteAlignment ) {
	s32 nBufBytes, nLoadedBytes;
	void *pBuf;
	FResFrame_t ResFrame;

	FASSERT( _bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );
	FASSERT( fresload_IsNameValid(pszResName) );

	ResFrame = fres_GetFrame();

	nBufBytes = fresload_GetObjectSizeOnDisk( psResType, pszResName );

	if( nBufBytes < 0 ) {
		// Could not get file size...

		if( pnSizeInBytes ) {
			*pnSizeInBytes = 0;
		}

		return NULL;
	}

	pBuf = fres_AlignedAlloc( (u32)nBufBytes, nByteAlignment );
	if( pBuf == NULL ) {
		// Could not allocate memory...

		if( pnSizeInBytes ) {
			*pnSizeInBytes = 0;
		}

		return NULL;
	}

	nLoadedBytes = fresload_LoadRaw( psResType, pszResName, pBuf, nBufBytes );
	if( nLoadedBytes < 0 ) {
		// Could not load object...

		fres_ReleaseFrame( ResFrame );

		if( pnSizeInBytes ) {
			*pnSizeInBytes = 0;
		}

		return NULL;
	}

	// Success loading object...

	if( pnSizeInBytes ) {
		*pnSizeInBytes = nBufBytes;
	}

	return pBuf;
}


static BOOL _FormPathName( char **ppDestPathName, const FResLoadReg_t *pRegistration, cchar *pszResName ) {
	return _FormPathName( ppDestPathName, pRegistration->sResType, pszResName, pRegistration->pszFileExtension );
}

static BOOL _FormPathName( char **ppDestPathName, cchar *psResType, cchar *pszResName, cchar *pszExtension ) {
	char *pszPathName;
	
	psResType; //MW bitches on unused paramaters...
	
	if( pszExtension == NULL ) {
		pszExtension = "";
	}

	pszPathName = (char *)fang_Malloc( _nPathLength + 1 + FRES_NAMELEN + 1 + fclib_strlen(pszExtension) + 1, 0 );
	if( pszPathName == NULL ) {
		return FALSE;
	}

	// Copy path...
	fclib_strcpy( pszPathName, _pszPath );
	if( pszPathName[_nPathLength-1] != _SLASH ) {
		pszPathName[_nPathLength] = _SLASH;
		pszPathName[_nPathLength+1] = 0;
	}

	fclib_strcat( pszPathName, pszResName );

	if( pszExtension[0] ) {
		fclib_strcat( pszPathName, "." );
		fclib_strcat( pszPathName, pszExtension );
	}

	*ppDestPathName = pszPathName;

	return TRUE;
}

static void _DestroyPathName( char *pszPathName ) {
	fang_Free( pszPathName );
}

static const FResLoadReg_t *_FindRegistration( cchar *psResType ) {
	u32 i;

	for( i=0; i<_nRegisteredHandlersCount; i++ ) {
		if( fres_CompareTypes( psResType, _apRegisteredHandlers[i]->sResType ) ) {
			// Found the desired type...
			return _apRegisteredHandlers[i];
		}
	}

	// Type not found...
	return NULL;
}

static void *_LoadResourceFromFile( cchar *pszPathName, const FResLoadReg_t *pRegistration, cchar *pszResName, cchar *psResType ) {
	FFileHandle hFile;
	s32 nFileBytes;
	void *pLoadedData;
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	FMemFrame_t MemFrameTemp = 0; 

	// Init stuff...
	pLoadedData = NULL;
	hFile = -1;
	ResFrame = fres_GetFrame();

	// Create the resource...
	hRes = fres_Create( pRegistration->sResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
		goto _LoadResourceExitWithError;
	}

	// Open the file...
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}
	hFile = ffile_Open( pszPathName, FFILE_OPEN_RONLY );
	if( hFile < 0 ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_NOT_FOUND );
		goto _LoadResourceExitWithError;
	}
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	// Determine the size of the file...
	nFileBytes = ffile_GetFileSize( hFile );
	if( nFileBytes <= 0 ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		goto _LoadResourceExitWithError;
	}

	// Allocate memory...
	if ( pRegistration->nMemType == FRESLOAD_MEMTYPE_TEMP || FResLoad_bLoadedForStreaming ) 
	{
		MemFrameTemp = fmem_GetFrame();
		pLoadedData = fmem_Alloc( (u32)nFileBytes, pRegistration->nAlignment );
		if( pLoadedData == NULL ) {
			_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
			goto _LoadResourceExitWithError;
		}
	}
	else if ( pRegistration->nMemType == FRESLOAD_MEMTYPE_PERM )
	{
		pLoadedData = fres_AlignedAlloc( (u32)nFileBytes, pRegistration->nAlignment );
		if( pLoadedData == NULL ) {
			_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
			goto _LoadResourceExitWithError;
		}
	}
	else if ( pRegistration->nMemType == FRESLOAD_MEMTYPE_CUST )
	{
		pLoadedData = pRegistration->pFcnAlloc( (u32)nFileBytes );
		if( pLoadedData == NULL ) {
			_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
			goto _LoadResourceExitWithError;
		}
	}
	else
	{
		FASSERT_NOW;
	}

	// Load data from disk...
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}
	if( ffile_Read( hFile, (u32)nFileBytes, pLoadedData ) < 0 ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		goto _LoadResourceExitWithError;
	}
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	// Close the file...
	ffile_Close( hFile );
	hFile = -1;

	// Convert GameCube byte order before platform resource handlers inspect data.
	if( !gcdata_Convert( pRegistration->pszFileExtension, pszResName, pLoadedData, (u32)nFileBytes ) ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_DATA_ERROR );
		goto _LoadResourceExitWithError;
	}

	// Call handler...
	if( pRegistration->pFcnCreate ) {
		if( !pRegistration->pFcnCreate( hRes, pLoadedData, (u32)nFileBytes, pszResName ) ) {
			// Handler failed...
			_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_DATA_ERROR );
			goto _LoadResourceExitWithError;
		}
	}
	FASSERT_MSG( fres_GetBase(hRes), "FResLoad::_LoadResourceFromFile: Handler forgot to set resource base." );

	// Free temporary memory...
	if( pRegistration->nMemType == FRESLOAD_MEMTYPE_TEMP || FResLoad_bLoadedForStreaming ) {
//		fang_Free( pLoadedData );
		fmem_ReleaseFrame(MemFrameTemp);
		MemFrameTemp = 0;
	}

	// Success...
	fres_SetCallback( hRes, pRegistration->pFcnDestroy );
	return fres_GetBase( hRes );

_LoadResourceExitWithError:
	if( hFile >= 0 ) {
		ffile_Close( hFile );
	}
	fres_ReleaseFrame( ResFrame );
	if( (pRegistration->nMemType == FRESLOAD_MEMTYPE_TEMP || FResLoad_bLoadedForStreaming) && MemFrameTemp) {
//		fang_Free( pLoadedData );
		fmem_ReleaseFrame(MemFrameTemp);
		MemFrameTemp = 0;
	}
	else if ( pRegistration->nMemType == FRESLOAD_MEMTYPE_CUST && pRegistration->pFcnFree )
	{
		pRegistration->pFcnFree( pLoadedData );
	}

	return NULL;
}

//
//
//
static FVisGeoHeader_t* _LoadMeshPortionOfWorld( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType ) 
{
	void *pMeshBase;
	FResFrame_t ResFrame;
	FResHandle_t hRes = NULL;
	const FResLoadReg_t *pMeshRegistration;
	u32 i;
	static FVisGeoHeader_t VisGeoHeader;
	u32 *pMeshSizes;
	u32 nInitialOffset;

	pMeshRegistration = _FindRegistration( FMESH_RESTYPE );
	if( pMeshRegistration == NULL ) 
	{
		return NULL;
	}

	char szMeshName[32];
	u32 nNameLength = fclib_strlen( pszResName );
	fclib_strcpy( szMeshName, pszResName );

	pMeshBase = NULL;
	ResFrame = fres_GetFrame();

	// Read the mesh init header...
	u32 nMeshInitBytes = sizeof( u32 ) * pWorldFileHeader->nNumMeshes;
	void *pMeshInit = fres_Alloc( nMeshInitBytes );
	if ( !pMeshInit )
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
		goto _LoadMeshPortionError;
	}
		
	if( _ReadFromFilePosition( hFile, pMeshInit, nMeshInitBytes, pWorldFileHeader->nOffsetToMeshInits ) < 0 )
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		fang_Free( pMeshInit );
		goto _LoadMeshPortionError;
	}

	// Read the mesh init header...
	pMeshSizes = (u32 *)fres_Alloc( nMeshInitBytes );
	if ( !pMeshSizes )
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
		goto _LoadMeshPortionError;
	}
		
	if( _ReadFromFilePosition( hFile, pMeshSizes, nMeshInitBytes, pWorldFileHeader->nOffsetToMeshSizes ) < 0 )
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		fang_Free( pMeshSizes );
		goto _LoadMeshPortionError;
	}

	// Setup the geo header	
	VisGeoHeader.nMeshCount = pWorldFileHeader->nNumMeshes;
	VisGeoHeader.papMeshArray = (FMesh_t **)pMeshInit;
	
	FResload_bLoadingWorld = TRUE;

	if ( pMeshRegistration->nMemType == FRESLOAD_MEMTYPE_TEMP )
	{
		// Create the mesh resources...
		nInitialOffset = (u32)VisGeoHeader.papMeshArray[0];
		for ( i = 0; i < VisGeoHeader.nMeshCount; i++ )
		{
			// Determine Mesh Size
			u32 nSize = pMeshSizes[i];
			
			FMemFrame_t MemFrame = fmem_GetFrame();
			
			// Allocate temporary memory to hold the mesh...
			pMeshBase = fmem_Alloc( nSize, pMeshRegistration->nAlignment );
			if( pMeshBase == NULL ) 
			{
				_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
				fmem_ReleaseFrame( MemFrame );
				goto _LoadMeshPortionError;
			}

			// Read in the mesh portion...
			if( _ReadFromFilePosition( hFile, pMeshBase, nSize, (u32)VisGeoHeader.papMeshArray[i] ) < 0 ) 
			{
				_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
				fmem_ReleaseFrame( MemFrame );
				goto _LoadMeshPortionError;
			}

			// Setup a unique name that reflects the world name and the volume idx
			szMeshName[nNameLength] = (char)(i/100) + '0';
			szMeshName[nNameLength+1] = (char)(i/10) + '0';
			szMeshName[nNameLength+2] = (char)(i%10) + '0';
			szMeshName[nNameLength+3] = 0;

			// Set the new mesh pointer if this memory was temporary
			hRes = fres_Create( FMESH_RESTYPE, szMeshName );
			if ( hRes == FRES_NULLHANDLE ) 
			{
				_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
				fmem_ReleaseFrame( MemFrame );
				goto _LoadMeshPortionError;
			}

			// Create the mesh...
			if( pMeshRegistration->pFcnCreate )
			{
				if( !pMeshRegistration->pFcnCreate( hRes, pMeshBase, nSize, szMeshName ) ) 
				{
					// Handler failed...
					_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_DATA_ERROR );
					fmem_ReleaseFrame( MemFrame );
					goto _LoadMeshPortionError;
				}
			}
			
			fmem_ReleaseFrame( MemFrame );

			// Set the new mesh pointer if this memory was temporary
			fres_SetCallback( hRes, pMeshRegistration->pFcnDestroy );

			// Fixup the mesh array pointer
			VisGeoHeader.papMeshArray[i] = (FMesh_t *)fres_GetBase(hRes);
		}

		// Success!
		
	}
	else
	{
		// Allocate permanent memory to hold the meshes...
		pMeshBase = fres_AlignedAlloc( pWorldFileHeader->nMeshBytes, pMeshRegistration->nAlignment );
		if( pMeshBase == NULL ) 
		{
			_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
			goto _LoadMeshPortionError;
		}

		// Read in the mesh portion...
		if( _ReadFromFilePosition( hFile, pMeshBase, pWorldFileHeader->nMeshBytes, (u32)VisGeoHeader.papMeshArray[0] ) < 0 ) 
		{
			_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
			goto _LoadMeshPortionError;
		}

		// Create the mesh resources...
		nInitialOffset = (u32)VisGeoHeader.papMeshArray[0];
		for ( i = 0; i < VisGeoHeader.nMeshCount; i++ )
		{
			// Determine Mesh Size
			u32 nSize = pMeshSizes[i];
			
			// Fixup the array of mesh pointers
			VisGeoHeader.papMeshArray[i] = (FMesh_t *)((u32)VisGeoHeader.papMeshArray[i] + (u32)pMeshBase - nInitialOffset );

			// Setup a unique name that reflects the world name and the volume idx
			szMeshName[nNameLength] = (char)(i/100) + '0';
			szMeshName[nNameLength+1] = (char)(i/10) + '0';
			szMeshName[nNameLength+2] = (char)i + '0';
			szMeshName[nNameLength+3] = 0;

			// Allocate a handle to a resourse
			hRes = fres_Create( FMESH_RESTYPE, szMeshName );
			if ( hRes == FRES_NULLHANDLE ) 
			{
				_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
				goto _LoadMeshPortionError;
			}

			// Create the mesh...
			if( pMeshRegistration->pFcnCreate )
			{
				if( !pMeshRegistration->pFcnCreate( hRes, VisGeoHeader.papMeshArray[i], nSize, szMeshName ) ) 
				{
					// Handler failed...
					_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_DATA_ERROR );
					goto _LoadMeshPortionError;
				}
			}
			
			//
			fres_SetCallback( hRes, pMeshRegistration->pFcnDestroy );
			VisGeoHeader.papMeshArray[i] = (FMesh_t *)fres_GetBase(hRes);
		}
	}

	// Success!
	
	fvis_ResolveLoadedVolumeMeshes( &VisGeoHeader );

	FResload_bLoadingWorld = FALSE;
	return &VisGeoHeader;

_LoadMeshPortionError:
	// Failure
	FResload_bLoadingWorld = FALSE;
	fres_ReleaseFrame( ResFrame );
	fang_Free( pMeshBase );
	return NULL;
}


//
//
//
static FVisData_t* _LoadWorldPortionOfWorld( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType ) 
{
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	FVisData_t* pWorld;

	ResFrame = fres_GetFrame();

	// Create the world resource...
	hRes = fres_Create( FWORLD_RESTYPE, pszResName );
	if( hRes == FRES_NULLHANDLE ) 
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
		goto _LoadWorldPortionError;
	}

	// Allocate perm memory to hold the world data
	pWorld = (FVisData_t *)fres_AlignedAlloc( pWorldFileHeader->nWorldBytes, FCLASS_BYTE_ALIGN );
	if( pWorld == NULL ) 
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
		goto _LoadWorldPortionError;
	}

	// Read in the world portion...
	if( _ReadFromFilePosition( hFile, pWorld, pWorldFileHeader->nWorldBytes, pWorldFileHeader->nWorldOffset ) < 0 ) 
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		goto _LoadWorldPortionError;
	}

	// Fixup the data...
	if ( !fvis_CreateVisData( hRes, pWorld ) )
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_DATA_ERROR );
		goto _LoadWorldPortionError;
	}
	
	// Success...

	return pWorld;

_LoadWorldPortionError:
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


//
//
//
static BOOL _LoadStreamingData( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType ) 
{
	// Make sure that there actually are shape inits in this world...
	if( pWorldFileHeader->nStreamingDataBytes == 0 ) {
		return TRUE;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();

	// Allocate temp memory to hold the world streaming data...
	void *pWorldStreamingData = fmem_Alloc( pWorldFileHeader->nStreamingDataBytes, 32 );
	if( pWorldStreamingData == NULL ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
		goto _ExitWithError;
	}

	// Read in the world init data...
	if( _ReadFromFilePosition( hFile, pWorldStreamingData, pWorldFileHeader->nStreamingDataBytes, pWorldFileHeader->nStreamingDataOffset ) < 0 ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		goto _ExitWithError;
	}

	fvis_InitWorldStreamingData( pWorldStreamingData, pWorldFileHeader->nStreamingDataBytes );
	
	// Free the temporary memory...
	fmem_ReleaseFrame( MemFrame );

	return TRUE;

	// Error:
_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}


//
//
//
static BOOL _InitWorldObjects( FFileHandle hFile, FData_WorldFileHeader_t *pWorldFileHeader, cchar *pszPathName, cchar *pszResName, cchar *psResType ) 
{
	FData_WorldInitHeader_t *pWorldInitHeader;
	CFWorldShapeInit *pShapeInitArray;

	// Make sure that there actually are shape inits in this world...
	if( pWorldFileHeader->nInitBytes == 0 ) {
		return TRUE;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();

	// Allocate temp memory to hold the world init data...
	pWorldInitHeader = (FData_WorldInitHeader_t *)fmem_Alloc( pWorldFileHeader->nInitBytes, FCLASS_BYTE_ALIGN );
	if( pWorldInitHeader == NULL ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_OUT_OF_MEMORY );
		goto _ExitWithError;
	}

	// Read in the world init data...
	if( _ReadFromFilePosition( hFile, pWorldInitHeader, pWorldFileHeader->nInitBytes, pWorldFileHeader->nInitOffsets ) < 0 ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		goto _ExitWithError;
	}

	// Init the world game objects...
	pShapeInitArray = (CFWorldShapeInit *)(pWorldInitHeader + 1);

	if( !fworld_CreateGameObjects( pszResName, pShapeInitArray, pShapeInitArray, pWorldInitHeader->nNumInitStructs ) ) {
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_DATA_ERROR );
		goto _ExitWithError;
	}

	// Free the temporary memory...
	fmem_ReleaseFrame( MemFrame );

	return TRUE;

	// Error:
_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}

static void *_LoadWorldResourceFromFile( cchar *pszPathName, cchar *pszResName, cchar *psResType ) {
	FFileHandle hFile;
	FData_WorldFileHeader_t WorldFileHeader;
	FResFrame_t ResFrame;
	FVisData_t *pWorld;
#if FHEAP_TRACK_MEM_ALLOCATIONS
	u16 nOldSecondaryLineNumber;
	cchar* pszOldSecondaryFileName;
#endif

#if FHEAP_TRACK_MEM_ALLOCATIONS
	//record these, and set them back at end of func
	nOldSecondaryLineNumber = Fheap_nSecondaryLineNumber;
	pszOldSecondaryFileName = Fheap_pszSecondaryFileName;
#endif

	CFTimer LoadTimer;
	LoadTimer.Reset();

	// Init stuff...
	ResFrame = fres_GetFrame();
	hFile = -1;

	// Open the file...

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}
	hFile = ffile_Open( pszPathName, FFILE_OPEN_RONLY );
	if ( hFile < 0 ) 
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_NOT_FOUND );
		goto _LoadWorldExitWithError;
	}
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}
	
	// Read the file header...
	if ( ffile_Read( hFile, sizeof(FData_WorldFileHeader_t), &WorldFileHeader ) < 0 ) 
	{
		_CallLogCallback( NULL, psResType, pszResName, pszPathName, FRESLOAD_LOGINFO_READ_ERROR );
		goto _LoadWorldExitWithError;
	}
	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

	DEVPRINTF( "---TIMING: _LoadWorldResourceFromFile() - Load WLD header: %f\n", LoadTimer.SampleSeconds( TRUE ) );

#if FHEAP_TRACK_MEM_ALLOCATIONS
	// Manually set the resource type for future allocations
	fclib_strcpy( Fheap_szResourceType, "" );
	Fheap_nSecondaryLineNumber = 0;
	Fheap_pszSecondaryFileName = NULL;
#endif

	// Tell everyone that we're just about to setup the world
	if ( !fworld_AnnouncePreLoad() ) 
	{
		// A module returned an error...
		goto _LoadWorldExitWithError;
	}

	DEVPRINTF( "---TIMING: _LoadWorldResourceFromFile() - fworld_AnnouncePreLoad(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

#if FHEAP_TRACK_MEM_ALLOCATIONS
	// Manually set the resource type for future allocations
	fclib_strcpy( Fheap_szResourceType, "WRLD" );
	Fheap_nSecondaryLineNumber = nOldSecondaryLineNumber;
	Fheap_pszSecondaryFileName = pszOldSecondaryFileName;
#endif

	// Create and load world resource...
	pWorld = _LoadWorldPortionOfWorld( hFile, &WorldFileHeader, pszPathName, pszResName, psResType );
	if ( pWorld == NULL ) 
	{
		goto _LoadWorldExitWithError;
	}

	DEVPRINTF( "---TIMING: _LoadWorldResourceFromFile() - _LoadWorldPortionOfWorld(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

#if FHEAP_TRACK_MEM_ALLOCATIONS
	// Manually set the resource type for future allocations
	fclib_strcpy( Fheap_szResourceType, "" );
	Fheap_nSecondaryLineNumber = 0;
	Fheap_pszSecondaryFileName = NULL;
#endif

	if ( !fworld_AnnouncePostLoad() ) 
	{
		// A module returned an error...
		fworld_AnnouncePostDestroy();
		goto _LoadWorldExitWithError;
	}
	
	DEVPRINTF( "---TIMING: _LoadWorldResourceFromFile() - fworld_AnnouncePostLoad(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

#if FHEAP_TRACK_MEM_ALLOCATIONS
	// Manually set the resource type for future allocations
	fclib_strcpy( Fheap_szResourceType, "MESH" );
	Fheap_nSecondaryLineNumber = nOldSecondaryLineNumber;
	Fheap_pszSecondaryFileName = pszOldSecondaryFileName;
#endif

	// Create the streaming data	
	if ( !_LoadStreamingData( hFile, &WorldFileHeader, pszPathName, pszResName, psResType ) )
	{
		goto _LoadWorldExitWithError;
	}
	
	DEVPRINTF( "---TIMING: _LoadWorldResourceFromFile() - _LoadStreamingData(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}
	// Create and load world meshes...
	if ( !_LoadMeshPortionOfWorld( hFile, &WorldFileHeader, pszPathName, pszResName, psResType ) )
	{
		goto _LoadWorldExitWithError;
	}

	DEVPRINTF( "---TIMING: _LoadWorldResourceFromFile() - _LoadMeshPortionOfWorld(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

#if FHEAP_TRACK_MEM_ALLOCATIONS
	// Manually set the resource type for future allocations
	fclib_strcpy( Fheap_szResourceType, "" );
	Fheap_nSecondaryLineNumber = 0;
	Fheap_pszSecondaryFileName = NULL;
#endif

	// Load world init data...
	if ( !_InitWorldObjects( hFile, &WorldFileHeader, pszPathName, pszResName, psResType ) ) 
	{
		goto _LoadWorldExitWithError;
	}

	DEVPRINTF( "---TIMING: _LoadWorldResourceFromFile() - _InitWorldObjects(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

	if( _pFcnProgressCallback ) {
		_pFcnProgressCallback( NULL );
	}

#if FHEAP_TRACK_MEM_ALLOCATIONS
	// Set the resource type back to WRLD
	fclib_strcpy( Fheap_szResourceType, "WRLD" );
	Fheap_nSecondaryLineNumber = nOldSecondaryLineNumber;
	Fheap_pszSecondaryFileName = pszOldSecondaryFileName;
#endif

	// Success...
	ffile_Close( hFile );
	return pWorld;

_LoadWorldExitWithError:
	// Set the resource type back to WRLD
#if FHEAP_TRACK_MEM_ALLOCATIONS
	fclib_strcpy( Fheap_szResourceType, "WRLD" );
	Fheap_nSecondaryLineNumber = nOldSecondaryLineNumber;
	Fheap_pszSecondaryFileName = pszOldSecondaryFileName;
#endif

	if( hFile >= 0 ) 
	{
		ffile_Close( hFile );
	}
	fres_ReleaseFrame( ResFrame );

	return NULL;
}

static s32 _ReadFromFilePosition( FFileHandle hFile, void *pDestBuf, u32 nBytesToRead, u32 nOffset ) {
	if( ffile_Seek( hFile, (s32)nOffset, FFILE_SEEK_SET) < 0 ) {
		return FALSE;
	}

	return ffile_Read( hFile, nBytesToRead, pDestBuf );
}

static void _CallLogCallback( void *pBase, cchar *psResType, cchar *pszResName, cchar *pszResPathName, FResLoadLogInfo_e nInfo ) {
	char szResType[FRES_TYPELEN+1];

	if( _pFcnLogCallback ) {
		szResType[FRES_TYPELEN] = 0;
		fclib_strncpy( szResType, psResType, FRES_TYPELEN );

		_pFcnLogCallback( pBase, szResType, pszResName, pszResPathName, nInfo );
	}
}

