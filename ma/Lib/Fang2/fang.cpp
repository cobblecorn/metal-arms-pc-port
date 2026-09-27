//////////////////////////////////////////////////////////////////////////////////////
// fang.cpp - Master Fang module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 09/25/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "fclib.h"
#if FANG_PLATFORM_DX
#include <memory.h>
#endif
#if !FANG_PLATFORM_GC
#include <malloc.h>
#endif
#include <stdio.h>
#include <string.h>

#include "fang.h"
#include "fmath.h"
#include "fres.h"

#if FANG_DETECT_MEMLEAKS
	#include "flinklist.h"
#endif

#if FANG_PLATFORM_DX
	#include "dx/fdx8.h"
	#include <signal.h>
	#include <stdio.h>
#elif FANG_PLATFORM_PS2
	#include "ps2/fps2.h"
#elif FANG_PLATFORM_GC
	#include "gc/fGC.h"
#endif


#if !FANG_DETECT_MEMLEAKS
	typedef struct {
		void *pMemBlock;								// Value returned by malloc()
	} _MallocInfo_t;
#else
	#define _MAX_MALLOCINFO_FILENAME	31

	typedef struct {
		void *pMemBlock;								// Value returned by malloc()

		char szFileName[_MAX_MALLOCINFO_FILENAME+1];	// Name of the file where fang_Malloc() was called
		int nLineNum;									// Line number of the file where fang_Malloc() was called
		u32 nBytesAlloc;								// Number of bytes that were allocated

		FLink_t Link;									// Link to other allocations
	} _MallocInfo_t;

	static FLinkRoot_t _MallocInfoRoot;
#endif


FangConfigDef_t Fang_ConfigDefs;
#if FANG_PLATFORM_WIN
BOOL Fang_bPortDiag = FALSE;
#endif


Fang_LaunchType_e Fang_nLaunchType;


BOOL Fang_bInitialized = FALSE;

static BOOL _bStartedUp;
static BOOL _bValidationEnabled;
static FResFrame_t _ResFrame;

cchar* fang_pszDevPrintLogFileName = NULL;		 //setting this to a FILE* on XB will cause XB to log all DevPrintf to the echo to the file.

#if FANG_DETECT_MEMLEAKS
	static void _FreeAll( void );
	static void _PrintMemLeakInfo( void );
#endif

static void *_Malloc( u32 nBytes, u32 nByteAlignment );


void fang_Init( void ) {
	u32 i;
	
	Fang_nLaunchType = FANG_LAUNCH_TYPE_UNKNOWN;

#if FANG_PLATFORM_PS2
	fps2_Init();
#endif

	for( i=0; i<sizeof(Fang_ConfigDefs); i++ ) {
		((u8 *)&Fang_ConfigDefs)[i] = 0;
	}

	// General:
	Fang_ConfigDefs.pFang_FcnPrintf = NULL;
	Fang_ConfigDefs.nRes_HeapBytes = (4*1024*1024);
	Fang_ConfigDefs.pRes_HeapStart = NULL;
	Fang_ConfigDefs.nViewport_MaxViewports = 16;
	Fang_ConfigDefs.bTex_D3DNoSysLock = FANG_DEBUG_BUILD ? TRUE : FALSE;

	// Sound memory	
	Fang_ConfigDefs.nAudio_MaxSoundBytes = (2047 * (4 * 1024));

	// Auxiliary memory
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 0;
	Fang_ConfigDefs.nAMem_SlowAuxiliaryMemoryBytes = 0;

	// Mesh:
	Fang_ConfigDefs.nMesh_MaxActiveLights = 100;
	Fang_ConfigDefs.nMesh_MaxCollSpheres = 100;

	// Coll:
	#if FANG_WINGC
		// PC campaign collision queries regularly exceed the console-era 250-impact limit.
		// A WEWRresrch3 query reached 1,046 impacts, and a WEWHchase01 one 2,077 (the chase's debris);
		// about 0.4 MB of impact records.
		Fang_ConfigDefs.nColl_MaxImpacts = 4096;
	#else
		Fang_ConfigDefs.nColl_MaxImpacts = 250;
	#endif

	// Xfm:
	Fang_ConfigDefs.nXfm_MaxModelStackXfms = 10;

	// World:
	Fang_ConfigDefs.nWorld_NodeCacheBytes = (10 * 1024);
	#if FANG_PLATFORM_PS2
		Fang_ConfigDefs.nWorld_MaxDesiredLightsPerNode = 1;
		Fang_ConfigDefs.nWorld_CollBufBytes = 50*1024;
	#elif FANG_PLATFORM_DX 
		Fang_ConfigDefs.nWorld_MaxDesiredLightsPerNode = 4;
		Fang_ConfigDefs.nWorld_CollBufBytes = 100*1024;
	#elif FANG_PLATFORM_GC
		Fang_ConfigDefs.nWorld_MaxDesiredLightsPerNode = 1;
		Fang_ConfigDefs.nWorld_CollBufBytes = 50*1024;
	#else
		#error <Fang Error: No valid platform defined.>
	#endif
	Fang_ConfigDefs.nWorld_MaxSortedObjectsPerNode = 200;
	Fang_ConfigDefs.nWorld_MaxLightsPerDraw = 500;
	Fang_ConfigDefs.nWorld_MaxMeshesPerDraw = 500;
	Fang_ConfigDefs.nWorld_MaxIntersects = 3500;

	// Renderer:
	Fang_ConfigDefs.nRenderer_StackBytes = 2048;

	// Root path
	Fang_ConfigDefs.pszFile_GameRootPathName = NULL;

	// File:
	Fang_ConfigDefs.pszFile_MasterFilePathName = NULL;

	// Movie:
	Fang_ConfigDefs.pszMovie_BasePathName = NULL;

	// PSprite:
	Fang_ConfigDefs.nPSprite_XBVBCount = 8;
	#if FANG_PLATFORM_XB
		Fang_ConfigDefs.nPSprite_D3DVBVertexCount = 256;
	#elif FANG_PLATFORM_WIN
		Fang_ConfigDefs.nPSprite_D3DVBVertexCount = 2048;
	#elif FANG_PLATFORM_PS2
		Fang_ConfigDefs.nPSprite_D3DVBVertexCount = 2048;
	#elif FANG_PLATFORM_GC
		Fang_ConfigDefs.nPSprite_D3DVBVertexCount = 256;
	#else
		#error <Fang Error: No valid platform defined.>
	#endif

	// Draw:
	Fang_ConfigDefs.nDraw_XBVBCount = 8;
	#if FANG_PLATFORM_XB
		Fang_ConfigDefs.nDraw_D3DVBVertexCount = 256;
	#elif FANG_PLATFORM_WIN
		Fang_ConfigDefs.nDraw_D3DVBVertexCount = 2048;
	#elif FANG_PLATFORM_PS2
		Fang_ConfigDefs.nDraw_D3DVBVertexCount = 2048;
	#elif FANG_PLATFORM_GC
		Fang_ConfigDefs.nDraw_D3DVBVertexCount = 256;
	#else
		#error <Fang Error: No valid platform defined.>
	#endif

	Fang_ConfigDefs.nShadow_D3DIndexBufferCount = 1000;

	Fang_ConfigDefs.nText_MaxCharsPerFrame = 500; //RAFHACK -- Bump up later, after FTEXT system implemented!
	Fang_ConfigDefs.nText_MaxCharsPerPrintf = 160;

	Fang_ConfigDefs.nPadio_MaxSamplesPerSec = 125;
	
	Fang_ConfigDefs.nFile_MaxSimultaneousFilesOpen = 32;

	Fang_ConfigDefs.nForce_MaxActiveEnvelopes = 16;

	Fang_ConfigDefs.nStringTable_DefaultMaxStringCountPerCluster = 50;
	Fang_ConfigDefs.bStringTable_CaseSensativeTableNames = FALSE;
	Fang_ConfigDefs.bStringTable_CaseSensativeStringNames = FALSE;

	Fang_ConfigDefs.nMaxParticleEmitters = 128;
	Fang_ConfigDefs.nMaxParticles = 3000;
	Fang_ConfigDefs.nMaxParticleEmitterSprites = 768;

	Fang_ConfigDefs.nVerlet_MaxCount = 250;
	Fang_ConfigDefs.fVerlet_MaxDeltaSecs = (1.0f / 30.0f);

	Fang_ConfigDefs.nVtxPool_VertexCount = 2500;

	Fang_ConfigDefs.bCheckPoint_StartupSystem = FALSE;
	Fang_ConfigDefs.bExceptionHandler_StartupSystem = FALSE;

	Fang_ConfigDefs.nLightGroup_nMaxVirtualLights = 16;

	Fang_ConfigDefs.nLightPool_nNumLights = 32;

#if FANG_WINGC
	// The consoles' budget ran out in busy fights (WEWHchase01: "No free vertices and couldn't reclaim
	// any", so bullet marks stopped appearing). Twice as many: they are drawn through fdraw's dynamic buffer.
	Fang_ConfigDefs.nMaxDecals = 400;
	Fang_ConfigDefs.nMaxDecalVertices = 6000;
#else
	Fang_ConfigDefs.nMaxDecals = 200;
	Fang_ConfigDefs.nMaxDecalVertices = 3000;
#endif

	_bValidationEnabled = FALSE;

	#if FANG_DETECT_MEMLEAKS
		flinklist_InitRoot( &_MallocInfoRoot, (s32)FANG_OFFSETOF( _MallocInfo_t, Link ) );
	#endif

	_bStartedUp = FALSE;
	Fang_bInitialized = TRUE;
}

BOOL fang_Startup( void ) {
	FASSERT( Fang_bInitialized );
	FASSERT( !_bStartedUp );

	#if FANG_PLATFORM_DX
		if( !fdx8_Startup() ) {
			return FALSE;
		}
	#elif FANG_PLATFORM_PS2
		if( !fps2_Startup() ) {
			return FALSE;
		}
	#elif FANG_PLATFORM_GC
		if( !fgc_Startup() ) {
			return FALSE;
		}
	#else
		#error <Fang Error: No valid platform defined.>
	#endif

	_bStartedUp = TRUE;

	_ResFrame = fres_GetFrame();

	return TRUE;
}

void fang_Shutdown( void ) {
	FASSERT( Fang_bInitialized );

	if( !_bStartedUp ) {
		return;
	}

	fres_ReleaseFrame( _ResFrame );

	#if FANG_PLATFORM_DX
		fdx8_Shutdown();
	#elif FANG_PLATFORM_PS2
		fps2_Shutdown();
	#elif FANG_PLATFORM_GC
		fgc_Shutdown();
	#else
		#error <Fang Error: No valid platform defined.>
	#endif

	#if FANG_DETECT_MEMLEAKS
		_PrintMemLeakInfo();
		_FreeAll();
	#endif

	_bStartedUp = FALSE;
}

BOOL fang_IsStartedUp( void ) {
//	FASSERT( _bInitialized ); // ayale 11/01/01: This will allow me to call fang_IsStartedUp() even if fang_Init() hasn't been called.
	return _bStartedUp;
}

void fang_ResetHardware( void ) {
#if FANG_PLATFORM_XB
	LAUNCH_DATA oLaunchData;
	fang_MemZero( &oLaunchData, sizeof( oLaunchData ) );

	LD_LAUNCH_DASHBOARD *pLaunchDashboard = (LD_LAUNCH_DASHBOARD *)( &oLaunchData );
	pLaunchDashboard->dwReason = XLD_LAUNCH_DASHBOARD_MAIN_MENU;

	XLaunchNewImage( NULL, &oLaunchData );
#elif FANG_PLATFORM_GC
	OSResetSystem( OS_RESET_RESTART, 0, FALSE );
#elif FANG_PLATFORM_WIN
#else
#endif
}

// Enables or disables the Validation flag which can be used by modules to enable
// special, very time intensive validation code.
// Returns the previous state.
// The Validation flag is FALSE by default.
BOOL fang_SetValidationState( BOOL bEnable ) {
	BOOL bOldEnable;

	FASSERT( Fang_bInitialized );

	bOldEnable = _bValidationEnabled;
	_bValidationEnabled = bEnable;

	return bOldEnable;
}

// Returns the current state of the Validation flag as set by fang_SetValidationState().
BOOL fang_GetValidationState( void ) {
	FASSERT( Fang_bInitialized );
	return _bValidationEnabled;
}

void *fang_MemCopy( void *pDst, const void *pSrc, u32 nBytes ) {
	u32 nWordCount;
	u8 *pnDst;
	const u8 *pnSrc;

	if( nBytes == 0 ) {
		return pDst;
	}

	pnSrc = (const u8 *)pSrc;
	pnDst = (u8 *)pDst;

#if FANG_PLATFORM_PS2
	if( (((u32)pDst | (u32)pSrc) & 15) == 0 ) {
		// Source and destination pointers are on an even 16-byte boundary...
		
		ALIGNED_MEMCPY16(pDst, pSrc, nBytes);
		pnDst += (nBytes & 0xFFFFFFF0);
		pnSrc += (nBytes & 0xFFFFFFF0);
		nBytes &= 15;
		
	} else if( (((u32)pDst | (u32)pSrc) & 7) == 0 ) {
		// Source and destination pointers are on an even 8-byte boundary...
		
		ALIGNED_MEMCPY8(pDst, pSrc, nBytes);
		pnDst += (nBytes & 0xFFFFFFF8);
		pnSrc += (nBytes & 0xFFFFFFF8);
		nBytes &= 7;
		
	} else if( (((u32)pDst | (u32)pSrc) & 3) == 0 ) {
#else
	if( (((u32)pDst | (u32)pSrc) & 3) == 0 ) {
#endif
		// Source and destination pointers are on an even 4-byte boundary...
		
		ALIGNED_MEMCPY4(pDst, pSrc, nBytes);
		pnDst += (nBytes & 0xFFFFFFFC);
		pnSrc += (nBytes & 0xFFFFFFFC);
		nBytes &= 3;
		
//ARG	} else if( (((u32)pDst | (u32)pSrc) & 2) == 0 ) {
	} else if( (((u32)pDst | (u32)pSrc) & 1) == 0 ) {	//ARG - BUG
		// Source and destination pointers are on an even 2-byte boundary...
		
		nWordCount = nBytes >> 1;
		nBytes &= 1;
		
		for( ; nWordCount; nWordCount-- ) {
			*(u16 *)pnDst = *(u16 *)pnSrc;
			pnDst += 2;
			pnSrc += 2;
		}
	}

	// Copy remaining bytes...
	for( ; nBytes; nBytes-- ) {
		*pnDst++ = *pnSrc++;
	}

	return pDst;
}

// Returns TRUE if the two memory blocks contain identical data.
// Returns FALSE otherwise.
//
// If nBytes is 0, returns TRUE.
BOOL fang_IsMemIdentical( const void *pSrc1, const void *pSrc2, u32 nBytes ) {
	u32 nWordCount;
	const u8 *pnSrc1, *pnSrc2;

	if( nBytes == 0 ) {
		return TRUE;
	}

	pnSrc1 = (const u8 *)pSrc2;
	pnSrc2 = (const u8 *)pSrc1;

	if( (((u32)pSrc1 | (u32)pSrc2) & 3) == 0 ) {
		// Source and destination pointers are on an even 4-byte boundary...

		nWordCount = nBytes >> 2;
		nBytes &= 3;

		for( ; nWordCount; nWordCount-- ) {
			if( *(u32 *)pnSrc2 != *(u32 *)pnSrc1 ) {
				return FALSE;
			}
			pnSrc1 += 4;
			pnSrc2 += 4;
		}
//ARG	} else if( (((u32)pSrc1 | (u32)pSrc2) & 2) == 0 ) {
	} else if( (((u32)pSrc1 | (u32)pSrc2) & 1) == 0 ) {	//ARG - BUG
		// Source and destination pointers are on an even 2-byte boundary...

		nWordCount = nBytes >> 1;
		nBytes &= 1;

		for( ; nWordCount; nWordCount-- ) {
			if( *(u16 *)pnSrc2 != *(u16 *)pnSrc1 ) {
				return FALSE;
			}
			pnSrc1 += 2;
			pnSrc2 += 2;
		}
	}

	// Compare remaining bytes...
	for( ; nBytes; nBytes-- ) {
		if( *pnSrc2++ != *pnSrc1++ ) {
			return FALSE;
		}
	}

	return TRUE;
}

void *fang_MemZero( void *pDst, u32 nBytes ) {
	u32 nWordCount;
	u8 *pnDst;

	if( nBytes == 0 ) {
		return pDst;
	}

	pnDst = (u8 *)pDst;

	if( ((u32)pDst & 3) == 0 ) {
		// Source pointer is on an even 4-byte boundary...

		nWordCount = nBytes >> 2;
		nBytes &= 3;

		for( ; nWordCount; nWordCount-- ) {
			*(u32 *)pnDst = 0;
			pnDst += 4;
		}
//ARG	} else if( ((u32)pDst & 2) == 0 ) {
	} else if( ((u32)pDst & 1) == 0 ) {	//ARG - BUG
		// Source pointer is on an even 2-byte boundary...

		nWordCount = nBytes >> 1;
		nBytes &= 1;

		for( ; nWordCount; nWordCount-- ) {
			*(u16 *)pnDst = 0;
			pnDst += 2;
		}
	}

	// Zero remaining bytes...
	for( ; nBytes; nBytes-- ) {
		*pnDst++ = 0;
	}

	return pDst;
}

void *fang_MemSet( void *pDst, u8 nValue, u32 nBytes ) {
	u32 nWordCount, nValue32;
	u16 nValue16;
	u8 *pnDst;

	if( nBytes == 0 ) {
		return pDst;
	}

	pnDst = (u8 *)pDst;

	if( ((u32)pDst & 3) == 0 ) {
		// Source pointer is on an even 4-byte boundary...

		nValue32 = (u32)( nValue | (nValue<<8) | (nValue<<16) | (nValue<<24) );

		nWordCount = nBytes >> 2;
		nBytes &= 3;

		for( ; nWordCount; nWordCount-- ) {
			*(u32 *)pnDst = nValue32;
			pnDst += 4;
		}
//ARG	} else if( ((u32)pDst & 2) == 0 ) {
	} else if( ((u32)pDst & 1) == 0 ) {	//ARG - BUG
		// Source pointer is on an even 2-byte boundary...

		nValue16 = (u16)( nValue | (nValue<<8) );

		nWordCount = nBytes >> 1;
		nBytes &= 1;

		for( ; nWordCount; nWordCount-- ) {
			*(u16 *)pnDst = nValue16;
			pnDst += 2;
		}
	}

	// Zero remaining bytes...
	for( ; nBytes; nBytes-- ) {
		*pnDst++ = nValue;
	}

	return pDst;
}

u64 fang_ConvertEndian( u64 num ) {
	u64	b1, b2, b3, b4, b5, b6, b7, b8;

	b1 = (num>>0)&0xff;
	b2 = (num>>8)&0xff;
	b3 = (num>>16)&0xff;
	b4 = (num>>24)&0xff;
	b5 = (num>>32)&0xff;
	b6 = (num>>40)&0xff;
	b7 = (num>>48)&0xff;
	b8 = (num>>56)&0xff;

	return ( (b1<<56) | (b2<<48) | (b3<<40) | (b4<<32) |
			 (b5<<24) | (b6<<16) | (b7<<8)  | b8 );
}

s64 fang_ConvertEndian( s64 num ) {
	u64 b1, b2, b3, b4, b5, b6, b7, b8;

	b1 = (u64)((num>>0)&0xff);
	b2 = (u64)((num>>8)&0xff);
	b3 = (u64)((num>>16)&0xff);
	b4 = (u64)((num>>24)&0xff);
	b5 = (u64)((num>>32)&0xff);
	b6 = (u64)((num>>40)&0xff);
	b7 = (u64)((num>>48)&0xff);
	b8 = (u64)((num>>56)&0xff);

	return (s64)( (b1<<56) | (b2<<48) | (b3<<40) | (b4<<32) |
				  (b5<<24) | (b6<<16) | (b7<<8)  | b8 );
}

f32 fang_ConvertEndian( f32 num ) {
	
	union
	{
		f32	f;
		u8	b[4];
	} dat1, dat2;
	
	dat1.f = num;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];

	return dat2.f;
}

u32 fang_ConvertEndian( u32 num ) {
	u32 b1, b2, b3, b4;

	b1 = (num>>0)&0xff;
	b2 = (num>>8)&0xff;
	b3 = (num>>16)&0xff;
	b4 = (num>>24)&0xff;

	return ( ((u32)b1<<24) | ((u32)b2<<16) | ((u32)b3<<8) | b4 );
}

s32 fang_ConvertEndian( s32 num ) {
	u32 b1, b2, b3, b4;

	b1 = (u32)((num>>0)&0xff);
	b2 = (u32)((num>>8)&0xff);
	b3 = (u32)((num>>16)&0xff);
	b4 = (u32)((num>>24)&0xff);

	return (s32)( (b1<<24) | (b2<<16) | (b3<<8) | b4 );
}

u16 fang_ConvertEndian( u16 num ) {
	u32 b1, b2;

	b1 = (u32)((num>>0)&0xff);
	b2 = (u32)((num>>8)&0xff);

	return (u16)( (b1<<8) | b2 );
}

s16 fang_ConvertEndian( s16 num ) {
	u32 b1,b2;

	b1 = (u32)((num>>0)&0xff);
	b2 = (u32)((num>>8)&0xff);

	return (s16)( (b1<<8) | b2 );
}

void* fang_ConvertEndian( void *ptr ) {
	u32 num = (u32)ptr;
	u32 b1, b2, b3, b4;

	b1 = (num>>0)&0xff;
	b2 = (num>>8)&0xff;
	b3 = (num>>16)&0xff;
	b4 = (num>>24)&0xff;

	return (void *)( (b1<<24) | (b2<<16) | (b3<<8) | b4 );
}

s8 fang_ConvertEndian( s8 num ) {
	return num;
}

u8 fang_ConvertEndian( u8 num ) { 
	return num;
}

//char fang_ConvertEndian( char num ) {
//	return num;   
//}


//------------------------------------------------------------------------------------------------------------------
// DevPrintf:
//------------------------------------------------------------------------------------------------------------------
#if FANG_ENABLE_DEVPRINTF
	#if FANG_PLATFORM_WIN

		#include <stdio.h>
		#include <tchar.h>
		#define _MAX_DEV_PRINT_STRING_LEN	255
		static char _aszDevPrintfString[_MAX_DEV_PRINT_STRING_LEN+1];

		int fang_DevPrintf( const char *pszFormat, ... ) {
			FANG_VA_LIST args;
			FANG_VA_START( args, pszFormat );

			if( Fang_ConfigDefs.pFang_FcnPrintf ) {
				Fang_ConfigDefs.pFang_FcnPrintf( pszFormat, args );
			}

			_vsnprintf( _aszDevPrintfString, _MAX_DEV_PRINT_STRING_LEN, pszFormat, args );
			_aszDevPrintfString[_MAX_DEV_PRINT_STRING_LEN] = 0;

#ifdef _UNICODE
			static TCHAR wszTemp[_MAX_DEV_PRINT_STRING_LEN+1];
			MultiByteToWideChar( CP_THREAD_ACP, MB_PRECOMPOSED, _aszDevPrintfString, strlen( _aszDevPrintfString ), wszTemp, sizeof( wszTemp ) );
			OutputDebugString( wszTemp );

			FANG_VA_END(args);

			return _tcslen( wszTemp );
#else
			OutputDebugString( _aszDevPrintfString );

			FANG_VA_END(args);

			return fclib_strlen( _aszDevPrintfString );
#endif
		}

	#elif FANG_PLATFORM_XB

		#define _MAX_DEV_PRINT_STRING_LEN	255
		static char _aszDevPrintfString[_MAX_DEV_PRINT_STRING_LEN+1];

		int fang_DevPrintf( const char *pszFormat, ... ) {
			FANG_VA_LIST args;
			FANG_VA_START( args, pszFormat );

			_vsnprintf( _aszDevPrintfString, _MAX_DEV_PRINT_STRING_LEN, pszFormat, args );
			_aszDevPrintfString[_MAX_DEV_PRINT_STRING_LEN] = 0;
			OutputDebugString( _aszDevPrintfString );


			if (fang_pszDevPrintLogFileName)
			{
				FILE* fp = fopen(fang_pszDevPrintLogFileName, "at+");
				if (fp)
				{
					fprintf( fp, _aszDevPrintfString );
					fclose(fp);
				}
				else
				{
					fang_pszDevPrintLogFileName = NULL;
				}
			}

			FANG_VA_END(args);

			return fclib_strlen( _aszDevPrintfString );
		}

	#endif

#else
	#if !FANG_PLATFORM_GC
		int fang_DevPrintf( const char *pszFormat, ... ) {
			return 0;
		}
	#endif	

#endif

//------------------------------------------------------------------------------------------------------------------
// Assertion handling:
//------------------------------------------------------------------------------------------------------------------
#if ! FANG_PLATFORM_WIN
	static volatile BOOL _Fang_bHang = TRUE;
#endif

void fang_Assert( cchar *pszFile, int nLine, cchar *pszExpr ) {
#if FANG_PLATFORM_WIN

	#define __MAX_MSG_LEN 200
	static char szMsgString[__MAX_MSG_LEN+1];
	int nReturnValue;
	DEVPRINTF( "*** FANG ASSERTION FAILURE *** File: %s Line: %i Expression: %s\n", pszFile, nLine, pszExpr );

	_snprintf( szMsgString, __MAX_MSG_LEN, "*** FANG ASSERTION FAILURE ***\n\nFile: %s\nLine: %i\nExpression: %s\n\nClick Ok to continue or Cancel to abort.", pszFile, nLine, pszExpr );

#ifdef _UNICODE
	static TCHAR wszTemp[__MAX_MSG_LEN+1];
	MultiByteToWideChar( CP_THREAD_ACP, MB_PRECOMPOSED, szMsgString, strlen( szMsgString ), wszTemp, sizeof( wszTemp ) );
	nReturnValue = MessageBox( NULL, wszTemp, _T("Fang Assertion Failure"), MB_ICONSTOP|MB_OKCANCEL );
#else
	nReturnValue = MessageBox( NULL, szMsgString, "Fang Assertion Failure", MB_ICONSTOP|MB_OKCANCEL );
#endif

	if( nReturnValue==0 || nReturnValue==IDCANCEL ) {
		// Abort the program...
		raise(SIGABRT);
		exit(3);
	}

	// Else, continue execution!
	#undef __MAX_MSG_LEN

#else

	printf("*** FANG ASSERTION FAILURE ***\nAssert in file %s on line %d:\n%s\nHanging the system...\n", pszFile, nLine, pszExpr);
	while(_Fang_bHang){}

#endif
}


//------------------------------------------------------------------------------------------------------------------
// Malloc/Free with alignment and leak support:
//------------------------------------------------------------------------------------------------------------------
#if !FANG_DETECT_MEMLEAKS
void *fang_Malloc( u32 nBytes, u32 nByteAlignment ) {
	void *pUserMemBlock;

	FASSERT( _bInitialized );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(int);
	}

	pUserMemBlock = _Malloc( nBytes, nByteAlignment );
	if( pUserMemBlock == NULL ) {
		return NULL;
	}

	return pUserMemBlock;
}

void *fang_MallocAndZero( u32 nBytes, u32 nByteAlignment ) {
	void *pUserMem;

	FASSERT( _bInitialized );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	pUserMem = fang_Malloc( nBytes, nByteAlignment );
	if( pUserMem == NULL ) {
		return NULL;
	}

	fang_MemZero( pUserMem, nBytes );

	return pUserMem;
}

void fang_Free( void *pMemBlock ) {
	_MallocInfo_t *pInfo;

	FASSERT( _bInitialized );

	if( pMemBlock ) {
		pInfo = (_MallocInfo_t *)pMemBlock - 1;
#if FANG_PLATFORM_GC
		OSFree( pInfo->pMemBlock );
#else
		free( pInfo->pMemBlock );
#endif
	}
}

#else

void *fang_MallocWithLeakDetection( cchar *pszFileName, int nLineNum, u32 nBytes, u32 nByteAlignment ) {
	void *pUserMemBlock;
	_MallocInfo_t *pInfo;
	cchar *pszName;

	FASSERT( Fang_bInitialized );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(int);
	}

	if( pszFileName && *pszFileName) {
		for( pszName=pszFileName; *pszName; pszName++ ) {}
		for( pszName--; *pszName!='\\' && *pszName!=':'; pszName-- ) {}
		pszName++;
	} else {
		pszName = "<Unknown>";
	}

	pUserMemBlock = _Malloc( nBytes, nByteAlignment );
	if( pUserMemBlock == NULL ) {
		return NULL;
	}

	pInfo = (_MallocInfo_t *)pUserMemBlock - 1;
	strncpy( pInfo->szFileName, pszName, _MAX_MALLOCINFO_FILENAME );
	pInfo->szFileName[_MAX_MALLOCINFO_FILENAME] = 0;
	pInfo->nLineNum = nLineNum;
	pInfo->nBytesAlloc = nBytes;

	flinklist_AddTail( &_MallocInfoRoot, pInfo );

	return pUserMemBlock;
}

void *fang_MallocAndZeroWithLeakDetection( cchar *pszFileName, int nLineNum, u32 nBytes, u32 nByteAlignment ) {
	void *pUserMem;

	FASSERT( Fang_bInitialized );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	pUserMem = fang_MallocWithLeakDetection( pszFileName, nLineNum, nBytes, nByteAlignment );
	if( pUserMem == NULL ) {
		return NULL;
	}

	fang_MemZero( pUserMem, nBytes );

	return pUserMem;
}

void fang_Free( void *pMemBlock ) {
	_MallocInfo_t *pInfo;

	FASSERT( Fang_bInitialized );

	if( pMemBlock ) {
		pInfo = (_MallocInfo_t *)pMemBlock - 1;
		flinklist_Remove( &_MallocInfoRoot, pInfo );
#if FANG_PLATFORM_GC
		OSFree( pInfo->pMemBlock );
#else
		free( pInfo->pMemBlock );
#endif
	}
}

static void _FreeAll( void ) {
	_MallocInfo_t *pInfo;

	while( (pInfo = (_MallocInfo_t *)flinklist_GetTail( &_MallocInfoRoot )) ) {
		flinklist_Remove( &_MallocInfoRoot, pInfo );
#if FANG_PLATFORM_GC
		OSFree( pInfo->pMemBlock );
#else
		free( pInfo->pMemBlock );
#endif
	}
}

static void _PrintMemLeakInfo( void ) {
	_MallocInfo_t *pInfo;

	for( pInfo=(_MallocInfo_t *)flinklist_GetHead( &_MallocInfoRoot  ); pInfo; pInfo=(_MallocInfo_t *)flinklist_GetNext( &_MallocInfoRoot, pInfo ) ) {
		DEVPRINTF( "*********************************************************\n" );
		DEVPRINTF( "**  Fang detected a memory leak:\n" );
		DEVPRINTF( "**\n" );
		DEVPRINTF( "**      Allocated by: %s\n", pInfo->szFileName );
		DEVPRINTF( "**       Line Number: %i\n", pInfo->nLineNum );
		DEVPRINTF( "**           Address: 0x%08x\n", (u32)(pInfo+1) );
		DEVPRINTF( "**        Byte Count: %i\n", pInfo->nBytesAlloc );
		DEVPRINTF( "*********************************************************\n" );
		DEVPRINTF( "\n" );
	}
}
#endif

static void *_Malloc( u32 nBytes, u32 nByteAlignment ) {
	u32 nBytesToAlloc;
	void *pMallocMemBlock, *pUserMemBlock;
	_MallocInfo_t *pInfo;

	FASSERT( fmath_IsPowerOf2( nByteAlignment, FALSE ) );

	FMATH_CLAMPMIN( nByteAlignment, sizeof(int) );

	nBytesToAlloc = nBytes + (nByteAlignment-1) + sizeof(_MallocInfo_t);

#if FANG_PLATFORM_GC
	pMallocMemBlock = OSAlloc( nBytesToAlloc );
#else
	pMallocMemBlock = malloc( nBytesToAlloc );
#endif
	if( pMallocMemBlock == NULL ) {
		return NULL;
	}

	pUserMemBlock = (void *)FMATH_BYTE_ALIGN_UP( (u32)pMallocMemBlock + sizeof(_MallocInfo_t), nByteAlignment );

	pInfo = (_MallocInfo_t *)pUserMemBlock - 1;
	FASSERT( (u32)pInfo >= (u32)pMallocMemBlock );

	pInfo->pMemBlock = pMallocMemBlock;

	return pUserMemBlock;
}


#if 0
//------------------------------------------------------------------------------------------------------------------
// GUID handling:
//------------------------------------------------------------------------------------------------------------------
void fang_GuidReset( void ) {
	FASSERT( _bInitialized );

}
#endif


