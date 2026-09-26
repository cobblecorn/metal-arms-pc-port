//////////////////////////////////////////////////////////////////////////////////////
// fresload.h - Fang resource loader module.
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

#ifndef _FRESLOAD_H_
#define _FRESLOAD_H_ 1

#include "fang.h"
#include "fres.h"


typedef enum {
	FRESLOAD_LOGINFO_LOADED_OK,			// The resource was loaded ok
	FRESLOAD_LOGINFO_ALREADY_LOADED,	// The resource has already been loaded
	FRESLOAD_LOGINFO_NOT_FOUND,			// The resource was not found on the media
	FRESLOAD_LOGINFO_READ_ERROR,		// The resource could not be read from the media
	FRESLOAD_LOGINFO_DATA_ERROR,		// The resource data could not be interpreted by the resource type handler
	FRESLOAD_LOGINFO_OUT_OF_MEMORY,		// Could not create resource because sufficient memory is not available

	FRESLOAD_LOGINFO_COUNT
} FResLoadLogInfo_e;


// Returns TRUE if successful, or FALSE if failed.
typedef void* FResLoadAllocCallback_t( u32 nAllocBytes );
typedef void FResLoadFreeCallback_t( void *pMemory );
typedef BOOL FResLoadCreateCallback_t( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
typedef void FResLoadLogCallback_t( void *pBase, cchar *pszResType, cchar *pszResName, cchar *pszResPathName, FResLoadLogInfo_e nInfo );

//typedef BOOL FResLoadProgressCallback_t( void );
typedef BOOL FResLoadProgressCallback_t( cchar* pszDebugString );

typedef enum {
	FRESLOAD_MEMTYPE_TEMP,	// Resource will be loaded into temporary memory via malloc(). The handler is responsible for relocating it to fres memory
	FRESLOAD_MEMTYPE_PERM,	// Resource will be loaded into permanent memory view fres_Alloc(). The handler must not move it (resizing is ok)
	FRESLOAD_MEMTYPE_CUST,  // Resource will be loaded into custom memory as provided by handler

	FRESLOAD_MEMTYPE_COUNT
} FResLoadMemType_t;


typedef struct {
	char sResType[FRES_TYPELEN];			// Resource type of handler (not NULL terminated)
	cchar *pszFileExtension;				// Filename extension of this type of object (must persist) (NULL=none)
	FResLoadMemType_t nMemType;				// Where ResLoad should load the object from disk
	u32 nAlignment;							// Alignment of memory when ResLoad loads the object from disk

	FResLoadFreeCallback_t *pFcnFree;		// Handler function called when memory is being freed
	FResLoadAllocCallback_t *pFcnAlloc;		// Handler function called when memory is being allocated to hold the object
	FResLoadCreateCallback_t *pFcnCreate;	// Handler function called once the resource has been loaded (NULL=none, but only if FRESLOAD_MEMTYPE_PERM used)
	FResDestroyCallback_t *pFcnDestroy;		// Handler function called when the object is being destroyed (NULL=none)
} FResLoadReg_t;


extern BOOL FResLoad_bLoadedForStreaming;

extern BOOL fresload_ModuleStartup( void );
extern void fresload_ModuleShutdown( void );

extern FResLoadLogCallback_t *fresload_SetLogCallback( FResLoadLogCallback_t *pLogCallback );
extern FResLoadLogCallback_t *fresload_GetLogCallback( void );

extern FResLoadProgressCallback_t *fresload_SetProgressCallback( FResLoadProgressCallback_t *pProgressCallback );
extern FResLoadProgressCallback_t *fresload_GetProgressCallback( void );

extern void fresload_SetPath( cchar *pszPath );
extern cchar *fresload_GetPath( void );

extern BOOL fresload_IsRegistered( cchar *psResType );
extern BOOL fresload_RegisterHandler( const FResLoadReg_t *pRegistration );

extern BOOL fresload_IsNameValid( cchar *pszResName );


#if FHEAP_TRACK_MEM_ALLOCATIONS
extern void *fresload_Load_Main( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, BOOL bForStreaming );
#define fresload_Load( psResType, pszResName ) fresload_Load_Main( __FILE__, __LINE__, psResType, pszResName, FALSE )
#define fresload_LoadStreaming( psResType, pszResName ) fresload_Load_Main( __FILE__, __LINE__, psResType, pszResName, TRUE )
#else
extern void *fresload_Load_Main( cchar *psResType, cchar *pszResName, BOOL bForStreaming );
#define fresload_Load( psResType, pszResName ) fresload_Load_Main( psResType, pszResName, FALSE )
#define fresload_LoadStreaming( psResType, pszResName ) fresload_Load_Main( psResType, pszResName, TRUE )
#endif

extern s32 fresload_GetObjectSizeOnDisk( cchar *psResType, cchar *pszResName );

extern BOOL fresload_ExistsOnDisk( cchar *psResType, cchar *pszResName );

extern s32 fresload_LoadRaw( cchar *psResType, cchar *pszResName, void *pDestBuffer, s32 nDestBufferBytes );
extern void *fresload_LoadRawIntoSysHeap( cchar *psResType, cchar *pszResName, s32 *pnSizeInBytes=NULL, u32 nByteAlignment=0 );
extern void *fresload_LoadRawIntoResMem( cchar *psResType, cchar *pszResName, s32 *pnSizeInBytes=NULL, u32 nByteAlignment=0 );



#endif

