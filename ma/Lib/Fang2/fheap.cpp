//////////////////////////////////////////////////////////////////////////////////////
// fheap.cpp - Fang heap module.
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
// 10/12/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fheap.h"
#include "fmath.h"
#include "fperf.h"
#include "fclib.h"


#if FHEAP_TRACK_MEM_ALLOCATIONS
	#include "ftex.h"

#if !FANG_PLATFORM_GC
#include <stdio.h>	 
#endif

	// PC platform memory limits
	#define _PC_TEXTURE_MEMORY_LIMIT	(18.f * 1024000.f)	//
	#define _PC_ANIM_MEMORY_LIMIT		(1.5f * 1024000.f)	//
	#define _PC_SOUND_MEMORY_LIMIT		(24.f * 1024000.f)	//
	// GameCube platform memory limits
	#define	_GC_MESH_MEMORY_LIMIT		(5.5f * 1024000.f)	// MB for mesh memory
	#define _GC_TEXTURE_MEMORY_LIMIT	(7.5f * 1024000.f)	// MB for texture memory
	#define _GC_ANIM_MEMORY_LIMIT		(1.5f * 1024000.f)	// MB for animation data
	#define _GC_SOUND_MEMORY_LIMIT		(8.f * 1024000.f)	// MB for sound data
	// XBox platform memory limits
	#define	_XB_MESH_MEMORY_LIMIT		(8.f * 1024000.f)	// MB for mesh memory
	#define _XB_TEXTURE_MEMORY_LIMIT	(18.f * 1024000.f)	// MB for texture memory
	#define _XB_ANIM_MEMORY_LIMIT		(1.5f * 1024000.f)	// MB for animation data
	#define _XB_SOUND_MEMORY_LIMIT		(8.f * 1024000.f)	// MB for sound data
//ARG - >>>>>
// PlayStation2 platform memory limits
//ARG - same as GC at the moment
	#define	_PS2_MESH_MEMORY_LIMIT		(5.5f * 1024000.f)	// MB for mesh memory
	#define _PS2_TEXTURE_MEMORY_LIMIT	(7.5f * 1024000.f)	// MB for texture memory
	#define _PS2_ANIM_MEMORY_LIMIT		(1.5f * 1024000.f)	// MB for animation data
	#define _PS2_SOUND_MEMORY_LIMIT		(8.f * 1024000.f)	// MB for sound data
//ARG - <<<<<

	#if FANG_PLATFORM_GC
		#define	_MESH_MEMORY_LIMIT		_GC_MESH_MEMORY_LIMIT
		#define _TEXTURE_MEMORY_LIMIT	_GC_TEXTURE_MEMORY_LIMIT
		#define _ANIM_MEMORY_LIMIT		_GC_ANIM_MEMORY_LIMIT
		#define _SOUND_MEMORY_LIMIT		_GC_SOUND_MEMORY_LIMIT
	#elif FANG_PLATFORM_XB || FANG_PLATFORM_WIN
		#define	_MESH_MEMORY_LIMIT		_XB_MESH_MEMORY_LIMIT
		#define _TEXTURE_MEMORY_LIMIT	_XB_TEXTURE_MEMORY_LIMIT
		#define _ANIM_MEMORY_LIMIT		_XB_ANIM_MEMORY_LIMIT
		#define _SOUND_MEMORY_LIMIT		_XB_SOUND_MEMORY_LIMIT
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		#define	_MESH_MEMORY_LIMIT		_PS2_MESH_MEMORY_LIMIT
		#define _TEXTURE_MEMORY_LIMIT	_PS2_TEXTURE_MEMORY_LIMIT
		#define _ANIM_MEMORY_LIMIT		_PS2_ANIM_MEMORY_LIMIT
		#define _SOUND_MEMORY_LIMIT		_PS2_SOUND_MEMORY_LIMIT
//ARG - <<<<<
	#endif

	char Fheap_szResourceType[8];
	cchar* Fheap_pszSecondaryFileName = NULL;
	u16 Fheap_nSecondaryLineNumber;
	MemTracker_t Fheap_nTrackerPool[FHEAP_MEM_TRACKERS_POOL_SIZE];
	u32 Fheap_nPoolCount = 0;
	u32 Fheap_nTotalMemTracked = 0;
	u32 Fheap_nTotalPCTexMemTracked = 0;
	u32 Fheap_nTotalXBTexMemTracked = 0;
	u32 Fheap_nTotalGCTexMemTracked = 0;
	u32 Fheap_nTotalPS2TexMemTracked = 0;	//ARG
	u32 Fheap_nTotalAnimMemTracked = 0;
	u32 Fheap_nTotalSoundMemTracked = 0;
	u32 Fheap_nTotalMeshMemTracked = 0;
	TexMemTracker_t Fheap_nTexTrackerPool[FHEAP_TEX_MEM_TRACKERS_POOL_SIZE];
	u32 Fheap_nTexPoolCount = 0;
	BOOL Fheap_bTrackingInvalid = FALSE;

	#if FHEAP_DETAILED_TRACKING

	#define _MAX_PRINT_STRING_LEN	255
	//#if FANG_PLATFORM_GC
	//static std::__tag_va_List _aszDumpPrintString[_MAX_PRINT_STRING_LEN+1];
	//#else
	static char _aszDumpPrintString[_MAX_PRINT_STRING_LEN+1];
	//#endif
	static void _DumpPrint( void* pDumpFile, const char *pszFormat, ... )
	{
#if FANG_PLATFORM_GC
		va_list args;
		va_start( args, pszFormat );
		_vsnprintf( _aszDumpPrintString, _MAX_PRINT_STRING_LEN, pszFormat, args );
#else
		FANG_VA_LIST args;
		FANG_VA_START( args, pszFormat );

		_vsnprintf( _aszDumpPrintString, _MAX_PRINT_STRING_LEN, pszFormat, args );
#endif
		_aszDumpPrintString[_MAX_PRINT_STRING_LEN] = 0;

#if FANG_PLATFORM_GC	 
		DEVPRINTF( _aszDumpPrintString );
#else
		if ( pDumpFile )
		{
			DEVPRINTF( _aszDumpPrintString );
			fprintf( (FILE *)pDumpFile, _aszDumpPrintString );
		}
		else
		{
			DEVPRINTF( _aszDumpPrintString );
		}
#endif
		FANG_VA_END(args);
	}

	//
	//
	//
	void fheap_ShowMemoryTracking( BOOL bShowModuleDetails, BOOL bShowByModule, BOOL bShowTexAllocs, BOOL bDumpToFile, cchar* pszFileName ) 
	{
		u32 i, j;
#if FANG_PLATFORM_GC	 
		void* hDumpFile = NULL;
#else
		FILE* hDumpFile = NULL;

		if (bDumpToFile && pszFileName)
		{
			char szTmpDupFileName[128];
			u32 uFileNameLen = fclib_strlen(pszFileName);
			if (uFileNameLen < 120)
			{
				fclib_strcpy(szTmpDupFileName, pszFileName);
				fclib_strcat(szTmpDupFileName, "00.csv");
				uFileNameLen+=6;
				u32 uCount = 0;
				FILE* fpTmp = NULL;
				while ((fpTmp = fopen(szTmpDupFileName,"r")) && uCount < 99)  //99 is infinite loop safety
				{
					fclose(fpTmp);
					uCount++;
					szTmpDupFileName[uFileNameLen-6] = '0'+(uCount%100/10);
					szTmpDupFileName[uFileNameLen-5] = '0'+(uCount%10)/1;
					szTmpDupFileName[uFileNameLen] = '\0';
				}
				hDumpFile = fopen(szTmpDupFileName,"wt");
			}
		}
#endif

		_DumpPrint(hDumpFile, "\nFHEAP:  HEAP ALLOCATIONS:\n\n");

		_DumpPrint(hDumpFile, "\nVERSION %d.%d.%d\n\n:", fversion_GetToolMajorVer(), fversion_GetToolMinorVer(), fversion_GetToolSubVer());

		_DumpPrint(hDumpFile, "TOTAL HEAP ALLOCATIONS:, %09d\n", Fheap_nTotalMemTracked);

		_DumpPrint(hDumpFile, "GEN:,%09d\n", Fheap_nTotalMemTracked);
#if FANG_PLATFORM_WIN
		_DumpPrint(	hDumpFile, "PCTEX:,%09d\n", Fheap_nTotalPCTexMemTracked);
#elif FANG_PLATFORM_XB
		_DumpPrint(	hDumpFile, "XBTEX:,%09d\n", Fheap_nTotalXBTexMemTracked);
#elif FANG_PLATFORM_GC
		_DumpPrint(	hDumpFile, "GCTEX:,%09d\n", Fheap_nTotalGCTexMemTracked);
#endif
		_DumpPrint(	hDumpFile, "ANIM:,%09d\n", Fheap_nTotalAnimMemTracked);
		_DumpPrint(	hDumpFile, "WDAT:,%09d\n", Fheap_nTotalSoundMemTracked);
		_DumpPrint(	hDumpFile, "MESH:,%09d\n\n\n", Fheap_nTotalMeshMemTracked);

#if FANG_PLATFORM_XB
		MEMORYSTATUS stat;
		GlobalMemoryStatus( &stat );
		_DumpPrint(	hDumpFile, "XB FREE:,%d\n\n\n", stat.dwAvailPhys );
#endif

		//
		//
		//	All Allocs in order
		//
		//
		_DumpPrint(hDumpFile, "FHEAP:  All Allocs In order\n\n");

		for ( i = 0; i < Fheap_nPoolCount; i++ )
		{
			Fheap_nTrackerPool[i].bConsidered = FALSE;
		}
		
		if (bShowModuleDetails)
		{
			for ( i = 0; i < Fheap_nPoolCount; i++ )
			{
				MemTracker_t *pTracker = &Fheap_nTrackerPool[i];
				if ( pTracker->bConsidered || pTracker->Frame == NULL )
				{
					continue;
				}
				
				u32 nMemAllocated = pTracker->nMemAllocated;
				
				for ( j = i + 1; j < Fheap_nPoolCount; j++ )
				{
					MemTracker_t *pTemp = &Fheap_nTrackerPool[j];
					
					if ( pTracker->bConsidered || pTracker->Frame == NULL )
					{
						continue;
					}
					
					if ( pTemp->nPrimaryLineNumber != pTracker->nPrimaryLineNumber 
						|| pTemp->nSecondaryLineNumber != pTracker->nSecondaryLineNumber )
					{
						continue;
					}
					
					if ( fclib_stricmp( pTemp->pszPrimaryFileName, pTracker->pszPrimaryFileName) != 0 
						|| fclib_stricmp( pTemp->szResourceType, pTracker->szResourceType) != 0 
						|| ((pTemp->pszSecondaryFileName && pTracker->pszSecondaryFileName) && fclib_stricmp( pTemp->pszSecondaryFileName, pTracker->pszSecondaryFileName) != 0 ))
					{
						continue;
					}
					
					nMemAllocated += pTemp->nMemAllocated;
					pTemp->bConsidered = TRUE;
				}

				cchar* pszSecondaryFileName = "";
				if (pTracker->pszSecondaryFileName)
				{
					pszSecondaryFileName = pTracker->pszSecondaryFileName;
				}
				_DumpPrint(hDumpFile, "%s,%d,%s,%d,%s,%d,%3.3f\n", pTracker->pszPrimaryFileName,			
																		pTracker->nPrimaryLineNumber,								
																		pszSecondaryFileName,										
																		pTracker->nSecondaryLineNumber,								
																		pTracker->szResourceType, nMemAllocated,					
																		((f32)nMemAllocated / (f32)Fheap_nTotalMemTracked) * 100.f);



							
				pTracker->bConsidered = TRUE;
			}
		}

		u32 nModuleAllocations = 0;
		_DumpPrint(hDumpFile, "\nFHEAP:  HEAP ALLOCATIONS BY MODULE:\n\n" );
		for ( i = 0; i < Fheap_nPoolCount; i++ )
		{
			Fheap_nTrackerPool[i].bConsidered = FALSE;
		}
		for ( i = 0; i < Fheap_nPoolCount; i++ )
		{
			MemTracker_t *pTracker = &Fheap_nTrackerPool[i];
			if ( pTracker->bConsidered || pTracker->Frame == NULL )
			{
				continue;
			}
			
			u32 nTotalMemory = pTracker->nMemAllocated;
			
			for ( j = i + 1; j < Fheap_nPoolCount; j++ )
			{
				MemTracker_t *pTemp = &Fheap_nTrackerPool[j];
				
				if ( pTracker->bConsidered || pTracker->Frame == NULL )
				{
					continue;
				}
				
				if ( fclib_stricmp( pTracker->pszPrimaryFileName, pTemp->pszPrimaryFileName ) == 0 
					&& fclib_stricmp( pTracker->szResourceType, pTemp->szResourceType ) == 0 )
				{
					pTemp->bConsidered = TRUE;
					nTotalMemory += pTemp->nMemAllocated;
				}
			}
			
			pTracker->bConsidered = TRUE;
			_DumpPrint(hDumpFile, "%s,%s,%d,%3.3f\n",	pTracker->pszPrimaryFileName,
															pTracker->szResourceType,
															nTotalMemory, ((f32)nTotalMemory / (f32)Fheap_nTotalMemTracked) * 100.f);
						
			nModuleAllocations += nTotalMemory;
		}
		_DumpPrint(hDumpFile, "\nFHEAP:  TOTAL MODULE HEAP ALLOCATIONS: %d\n\n", nModuleAllocations );
		
		if (bShowTexAllocs)
		{
			_DumpPrint(hDumpFile, "\nFHEAP:  TEXTURE IMAGE HEAP ALLOCATIONS:\n\n" );
			for ( i = 0; i < Fheap_nTexPoolCount; i++ )
			{
				 
#if FANG_PLATFORM_WIN																						
				u32 uMemAllocated =	Fheap_nTexTrackerPool[i].nPCMemAllocated;	
#elif FANG_PLATFORM_XB																						
				u32 uMemAllocated = Fheap_nTexTrackerPool[i].nXBMemAllocated;
#elif FANG_PLATFORM_GC											
				u32 uMemAllocated =	Fheap_nTexTrackerPool[i].nGCMemAllocated;
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
				u32 uMemAllocated =	Fheap_nTexTrackerPool[i].nGCMemAllocated;
//ARG - <<<<<
#endif
				_DumpPrint(hDumpFile, "%s,%s,%d x %d,%d,%s,%d\n", ((FTexInfo_t *)Fheap_nTexTrackerPool[i].pTexInfo)->szName, 
										   Fheap_nTexTrackerPool[i].pszRequestingModuleName ? Fheap_nTexTrackerPool[i].pszRequestingModuleName : "NULL",
										   ((FTexInfo_t *)Fheap_nTexTrackerPool[i].pTexInfo)->nTexelsAcross,							
										   ((FTexInfo_t *)Fheap_nTexTrackerPool[i].pTexInfo)->nTexelsDown,								
										   ((FTexInfo_t *)Fheap_nTexTrackerPool[i].pTexInfo)->nLodCount,								
										   Fheap_nTexTrackerPool[i].pszFormatName,														
										   uMemAllocated);


			}
	
		}
#if !FANG_PLATFORM_GC
		if (hDumpFile) 
		{
			fclose(hDumpFile);
		}
#endif
	}

void fheap_ShowMemoryTracking( void ) 
{
	fheap_ShowMemoryTracking(FALSE,//BOOL bShowModuleDetails,
							 TRUE,//BOOL bShowByModule, 
							 TRUE,//BOOL bShowTexAllocs,
							 FALSE,//BOOL bDumpToFile,
							 NULL ); 
}

#endif // FHEAP_DETAILED_TRACKING


	//
	//
	//
	void fheap_TrackMem( cchar *pszFileName, u32 nLineNum, u32 nBytes )
	{
		MemTracker_t *pTracker = NULL;

		if ( Fheap_nPoolCount >= FHEAP_MEM_TRACKERS_POOL_SIZE )
		{
			if ( !Fheap_bTrackingInvalid )
			{
				DEVPRINTF( "fheap_TrackMem() - Exceeded Mem Trackers pool.  Memory Tracking will be inaccurate.\n" );
			}
			Fheap_bTrackingInvalid = TRUE;
		}
		else
		{
			pTracker = &Fheap_nTrackerPool[Fheap_nPoolCount++];

			// Record allocation information		
		#if FHEAP_DETAILED_TRACKING
			pTracker->pszPrimaryFileName = pszFileName;
			pTracker->nPrimaryLineNumber = (u16)nLineNum;
			pTracker->pszSecondaryFileName = Fheap_pszSecondaryFileName;
			pTracker->nSecondaryLineNumber = Fheap_nSecondaryLineNumber;
		#endif // FHEAP_DETAILED TRACKING
			fclib_strcpy( pTracker->szResourceType, Fheap_szResourceType );
			pTracker->nMemAllocated = nBytes;
			pTracker->Frame = fres_GetFrame();
		}
		
		Fheap_nTotalMemTracked += nBytes;

		f32 fRatio;
		
		u32 nTrackedMem = Fheap_nTotalMemTracked;
		if ( Fheap_bTrackingInvalid )
		{
			nTrackedMem = 0;
		}
		fperf_Render_AddPerfFloat( FPERF_TYPE_MEMORY, "KB   :", (f32)FRes_CFHeap.GetFreeBytes() / 1024.f, -1.f );

#if FANG_PLATFORM_XB
		MEMORYSTATUS stat;
		GlobalMemoryStatus( &stat );

		fperf_Render_AddPerfFloat( FPERF_TYPE_MEMORY, "XB KB:", (f32)((f32)stat.dwAvailPhys - (f32)(64 * 1024 * 1024)) / 1024.f, -1.f );
#endif

		fRatio = (f32)Fheap_nTotalMemTracked * (1.f / Fang_ConfigDefs.nRes_HeapBytes);
		fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "GEN", fRatio, -1.f );
		
		if ( fclib_stricmp( Fheap_szResourceType, "ANIM" ) == 0 )
		{
			Fheap_nTotalAnimMemTracked += nBytes;
			fRatio = (f32)Fheap_nTotalAnimMemTracked * (1.f / _ANIM_MEMORY_LIMIT);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "ANIM", fRatio, -1.f );
		}
		else if ( fclib_stricmp( Fheap_szResourceType, "WDAT" ) == 0 )
		{
			Fheap_nTotalSoundMemTracked += nBytes;
			fRatio = (f32)Fheap_nTotalSoundMemTracked * (1.f / Fang_ConfigDefs.nAudio_MaxSoundBytes);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "SND", fRatio, -1.f );
		}
		else if (  fclib_stricmp( Fheap_szResourceType, "MESH" ) == 0 
				|| fclib_stricmp( Fheap_szResourceType, "WRLD" ) == 0  )
		{
			Fheap_nTotalMeshMemTracked += nBytes;
			fRatio = (f32)Fheap_nTotalMeshMemTracked * (1.f / _MESH_MEMORY_LIMIT);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "MESH", fRatio, -1.f );
		}
		else if ( fclib_stricmp( Fheap_szResourceType, "TXTR" ) == 0 )
		{
			#if FANG_PLATFORM_WIN
				fRatio = (f32)Fheap_nTotalPCTexMemTracked * (1.f / _PC_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "PCTex", fRatio, -1.f );
				fRatio = (f32)Fheap_nTotalXBTexMemTracked * (1.f / _XB_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "XBTex", fRatio, -1.f );
				fRatio = (f32)Fheap_nTotalGCTexMemTracked * (1.f / _GC_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "GCTex", fRatio, -1.f );
			#elif FANG_PLATFORM_XB
				fRatio = (f32)Fheap_nTotalXBTexMemTracked * (1.f / _XB_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "XBTex", fRatio, -1.f );
			#elif FANG_PLATFORM_GC
				fRatio = (f32)Fheap_nTotalGCTexMemTracked * (1.f / _GC_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "GCTex", fRatio, -1.f );
			#endif
		}
		else
		{
			#if FANG_PLATFORM_WIN
				fRatio = (f32)Fheap_nTotalPCTexMemTracked * (1.f / _PC_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "PCTex", fRatio, -1.f );
				fRatio = (f32)Fheap_nTotalXBTexMemTracked * (1.f / _XB_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "XBTex", fRatio, -1.f );
				fRatio = (f32)Fheap_nTotalGCTexMemTracked * (1.f / _GC_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "GCTex", fRatio, -1.f );
			#elif FANG_PLATFORM_XB
				fRatio = (f32)Fheap_nTotalXBTexMemTracked * (1.f / _XB_TEXTURE_MEMORY_LIMIT);
				fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "XBTex", fRatio, -1.f );
			#endif
		}
	}

	
	//
	//
	//
	void fheap_UnTrackMem( FHeapFrame_t Frame )
	{
		u32 i;
		MemTracker_t *pTracker = NULL;
		for ( i = 0; i < Fheap_nPoolCount; i++ )
		{
			pTracker = &Fheap_nTrackerPool[i];
			
			if ( pTracker->Frame == NULL )
			{
				continue;
			}
			
			if ( (u32)pTracker->Frame > (u32)Frame )
			{
				continue;
			}
			
			if ( fclib_stricmp( pTracker->szResourceType, "ANIM" ) == 0 )
			{
				Fheap_nTotalAnimMemTracked -= pTracker->nMemAllocated;
			}
			else if ( fclib_stricmp( pTracker->szResourceType, "WDAT" ) == 0 )
			{
				Fheap_nTotalSoundMemTracked -= pTracker->nMemAllocated;
			}
			else if (  fclib_stricmp( pTracker->szResourceType, "MESH" ) == 0
					|| fclib_stricmp( pTracker->szResourceType, "WRLD" ) == 0 )
			{
				Fheap_nTotalMeshMemTracked -= pTracker->nMemAllocated;
			}
			
			Fheap_nTotalMemTracked -= pTracker->nMemAllocated;
			
			pTracker->Frame = NULL;
		}
		
		// Close the gaps left in the list
		for ( i = 0; i < Fheap_nPoolCount; i++ )
		{
			if ( Fheap_nTrackerPool[i].Frame == NULL )
			{
				// Copy the last tracker over this one
				Fheap_nTrackerPool[i] = Fheap_nTrackerPool[Fheap_nPoolCount - 1];
				Fheap_nPoolCount--;
			}
		}
		
		u32 nTrackedMem = Fheap_nTotalMemTracked;
		if ( Fheap_bTrackingInvalid )
		{
			nTrackedMem = 0;
		}
		fperf_Render_AddPerfFloat( FPERF_TYPE_MEMORY, "KB   :", (f32)FRes_CFHeap.GetFreeBytes() / 1024.f, -1.f );
		
		f32 fRatio = (f32)Fheap_nTotalMemTracked * (1.f / Fang_ConfigDefs.nRes_HeapBytes);
		fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "GEN", fRatio, -1.f );
		fRatio = (f32)Fheap_nTotalAnimMemTracked * (1.f / _ANIM_MEMORY_LIMIT);
		fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "ANIM", fRatio, -1.f );
		fRatio = (f32)Fheap_nTotalSoundMemTracked * (1.f / Fang_ConfigDefs.nAudio_MaxSoundBytes);
		fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "SND", fRatio, -1.f );
		fRatio = (f32)Fheap_nTotalMeshMemTracked * (1.f / _MESH_MEMORY_LIMIT);
		fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "MESH", fRatio, -1.f );
	
		#if FANG_PLATFORM_WIN
			fRatio = (f32)Fheap_nTotalPCTexMemTracked * (1.f / _PC_TEXTURE_MEMORY_LIMIT);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "PCTex", fRatio, -1.f );
			fRatio = (f32)Fheap_nTotalXBTexMemTracked * (1.f / _XB_TEXTURE_MEMORY_LIMIT);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "XBTex", fRatio, -1.f );
			fRatio = (f32)Fheap_nTotalGCTexMemTracked * (1.f / _GC_TEXTURE_MEMORY_LIMIT);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "GCTex", fRatio, -1.f );
		#elif FANG_PLATFORM_XB
			fRatio = (f32)Fheap_nTotalXBTexMemTracked * (1.f / _XB_TEXTURE_MEMORY_LIMIT);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "XBTex", fRatio, -1.f );
		#elif FANG_PLATFORM_GC
			fRatio = (f32)Fheap_nTotalGCTexMemTracked * (1.f / _GC_TEXTURE_MEMORY_LIMIT);
			fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "GCTex", fRatio, -1.f );
		#endif
	}
	
	//
	//
	//
	void fheap_UnTrackTexMem( void *pTexInfo )
	{
		u32 i;
		for ( i = 0; i < Fheap_nTexPoolCount; i++ )
		{
			if ( Fheap_nTexTrackerPool[i].pTexInfo != NULL
				&& Fheap_nTexTrackerPool[i].pTexInfo == pTexInfo )
			{
				Fheap_nTotalXBTexMemTracked -= Fheap_nTexTrackerPool[i].nXBMemAllocated;
				Fheap_nTotalGCTexMemTracked -= Fheap_nTexTrackerPool[i].nGCMemAllocated;
				Fheap_nTotalPCTexMemTracked -= Fheap_nTexTrackerPool[i].nPCMemAllocated;
				Fheap_nTotalPS2TexMemTracked -= Fheap_nTexTrackerPool[i].nPS2MemAllocated;	//ARG
				Fheap_nTexTrackerPool[i].pTexInfo = NULL;
				if ( i != Fheap_nTexPoolCount - 1 )
				{
					Fheap_nTexTrackerPool[i] = Fheap_nTexTrackerPool[Fheap_nTexPoolCount - 1];
				}
				Fheap_nTexPoolCount--;
				return;
			}
		}
		
//		DEVPRINTF( "Unable to find a texture allocated that matches %x\n", pTexInfo );
//		FASSERT_NOW;
	}
#endif // FHEAP_TRACK_MEM_ALLOCATIONS	


CFHeap::CFHeap() {
	m_bWeAlloced = FALSE;
	m_nHeapBytes = 0;
	m_anHeapExtents[0] = 0;
	m_anHeapExtents[1] = 0;
	m_anHeap[0] = 0;
	m_anHeap[1] = 0;
}

CFHeap::CFHeap( u32 nBytes, void *pMemBase ) {
	m_nHeapBytes = 0;
	Init( nBytes, pMemBase );
}

CFHeap::~CFHeap() {
	Free();
}

BOOL CFHeap::Init( u32 nBytes, void *pMemBase ) {
	FASSERT( !m_nHeapBytes );
	FASSERT( nBytes );

	m_nHeapBytes = nBytes;
	if( pMemBase ) {
		m_anHeapExtents[0] = (u32)pMemBase;
		m_bWeAlloced = FALSE;
	} else {
		m_anHeapExtents[0] = (u32)fang_Malloc( nBytes, 0 );
		m_bWeAlloced = TRUE;
	}

	if( m_anHeapExtents[0] == 0 ) {
		m_bWeAlloced = FALSE;
		m_nHeapBytes = 0;
		m_anHeapExtents[0] = 0;
		m_anHeapExtents[1] = 0;
		m_anHeap[0] = 0;
		m_anHeap[1] = 0;

#if FANG_PLATFORM_GC
		OSReport( "CFHeap::Init() - FANG HEAP ALLOCATION OF %d KBs FAILED!!!\n", (nBytes + 1024) / 1024 );
#endif
		return FALSE;
	}

	m_anHeapExtents[1] = m_anHeapExtents[0] + m_nHeapBytes;

	m_anHeap[0] = m_anHeapExtents[0];
	m_anHeap[1] = m_anHeapExtents[1];

#if FHEAP_TRACK_MEM_ALLOCATIONS
	Fheap_szResourceType[0] = 0;
	Fheap_pszSecondaryFileName = NULL;
	Fheap_nSecondaryLineNumber = 0;
	Fheap_nPoolCount = 0;
	Fheap_nTotalMemTracked = 0;
	Fheap_nTexPoolCount = 0;
	Fheap_nTotalPCTexMemTracked = 0;
	Fheap_nTotalXBTexMemTracked = 0;
	Fheap_nTotalGCTexMemTracked = 0;
	Fheap_nTotalPS2TexMemTracked = 0;	//ARG
	Fheap_nTotalAnimMemTracked = 0;
	Fheap_nTotalSoundMemTracked = 0;
	Fheap_nTotalMeshMemTracked = 0;
	Fheap_bTrackingInvalid = FALSE;
	
	u32 i;
	for ( i = 0; i < FHEAP_MEM_TRACKERS_POOL_SIZE; i++ )
	{
		Fheap_nTrackerPool[i].Frame = NULL;
	}
#endif // FHEAP_TRACK_MEM_ALLOCATIONS

	return TRUE;
}

void CFHeap::Free( void ) {
	if( m_nHeapBytes ) {
		if( m_bWeAlloced ) {
			fang_Free( (void *)m_anHeapExtents[0] );
		}

		m_bWeAlloced = FALSE;
		m_nHeapBytes = 0;
		m_anHeapExtents[0] = 0;
		m_anHeapExtents[1] = 0;
		m_anHeap[0] = 0;
		m_anHeap[1] = 0;
	}
}

void *CFHeap::Alloc( u32 nHeapNum, u32 nBytes, u32 nByteAlignment ) {
	FASSERT( m_nHeapBytes );
	FASSERT( nHeapNum < 2 );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nHeapNum ) {
		// Upper heap...
		return AllocHeap1( nBytes, nByteAlignment );
	} else {
		// Lower heap...
		return AllocHeap0( nBytes, nByteAlignment );
	}
}

void *CFHeap::AllocAndZero( u32 nHeapNum, u32 nBytes, u32 nByteAlignment ) {
	FASSERT( m_nHeapBytes );
	FASSERT( nHeapNum < 2 );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nHeapNum ) {
		// Upper heap...
		return AllocAndZeroHeap1( nBytes, nByteAlignment );
	} else {
		// Lower heap...
		return AllocAndZeroHeap0( nBytes, nByteAlignment );
	}
}

void CFHeap::ReleaseFrame( u32 nHeapNum, FHeapFrame_t Frame ) {
	u32 nFrameByte;

	FASSERT( m_nHeapBytes );

	if( Frame ) {
		FASSERT( nHeapNum < 2 );

		nFrameByte = (u32)Frame;

		FASSERT( nHeapNum==1 || (nFrameByte<=m_anHeap[0] && nFrameByte>=m_anHeapExtents[0]) );
		FASSERT( nHeapNum==0 || (nFrameByte>=m_anHeap[1] && nFrameByte<=m_anHeapExtents[1]) );

		#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
			if( nHeapNum == 1 ) {
				fang_MemSet( (void *)m_anHeap[1], 0xCD, nFrameByte - m_anHeap[1] );
			} else {
				fang_MemSet( (void *)nFrameByte, 0xCD, m_anHeap[0] - nFrameByte );
			}
		#endif

		m_anHeap[nHeapNum] = nFrameByte;
	}
}

void CFHeap::ReleaseFrameHeap0( FHeapFrame_t Frame ) {
	u32 nFrameByte;

	FASSERT( m_nHeapBytes );

	if( Frame ) {
		nFrameByte = (u32)Frame;

		FASSERT( nFrameByte<=m_anHeap[0] && nFrameByte>=m_anHeapExtents[0] );

		#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
			fang_MemSet( (void *)nFrameByte, 0xCD, m_anHeap[0] - nFrameByte );
		#endif

		m_anHeap[0] = nFrameByte;
	}
}

void CFHeap::ReleaseFrameHeap1( FHeapFrame_t Frame ) {
	u32 nFrameByte;

	FASSERT( m_nHeapBytes );

	if( Frame ) {
		nFrameByte = (u32)Frame;

		FASSERT( nFrameByte>=m_anHeap[1] && nFrameByte<=m_anHeapExtents[1] );

		#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
			fang_MemSet( (void *)m_anHeap[1], 0xCD, nFrameByte - m_anHeap[1] );
		#endif

		m_anHeap[1] = nFrameByte;
	}
}

void *CFHeap::AllocHeap0( u32 nBytes, u32 nByteAlignment ) {
	u32 nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nAlignedMem = FMATH_BYTE_ALIGN_UP( m_anHeap[0], nByteAlignment );

	if( (nAlignedMem + nBytes) <= m_anHeap[1] ) {
		m_anHeap[0] = nAlignedMem + nBytes;
		return (void *)nAlignedMem;
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocHeap0() - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocHeap1( u32 nBytes, u32 nByteAlignment ) {
	u32 nOffsetBytes, nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nOffsetBytes = m_anHeap[1] - nBytes;
	nAlignedMem = FMATH_BYTE_ALIGN_DOWN( nOffsetBytes, nByteAlignment );

	if( nAlignedMem >= m_anHeap[0] ) {
		m_anHeap[1] = nAlignedMem;
		return (void *)nAlignedMem;
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocHeap1() - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocAndZeroHeap0( u32 nBytes, u32 nByteAlignment ) {
	u32 nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nAlignedMem = FMATH_BYTE_ALIGN_UP( m_anHeap[0], nByteAlignment );

	if( (nAlignedMem + nBytes) <= m_anHeap[1] ) {
		m_anHeap[0] = nAlignedMem + nBytes;
		fang_MemZero( (void *)nAlignedMem, nBytes );
		return (void *)nAlignedMem;
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocAndZeroHeap0() - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocAndZeroHeap1( u32 nBytes, u32 nByteAlignment ) {
	u32 nOffsetBytes, nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nOffsetBytes = m_anHeap[1] - nBytes;
	nAlignedMem = FMATH_BYTE_ALIGN_DOWN( nOffsetBytes, nByteAlignment );

	if( nAlignedMem >= m_anHeap[0] ) {
		m_anHeap[1] = nAlignedMem;
		fang_MemZero( (void *)nAlignedMem, nBytes );
		return (void *)nAlignedMem;
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocAndZeroHeap1() - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocHeap0( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	u32 nOffsetHeap, nOffsetBytes, nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );
	FASSERT( fmath_IsPowerOf2( nOffset, TRUE ) );
	FASSERT( nOffset <= nBytes );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nOffsetBytes = nBytes - nOffset;
	nOffsetHeap = m_anHeap[0] + nOffset;
	nAlignedMem = FMATH_BYTE_ALIGN_UP( nOffsetHeap, nByteAlignment );

	if( (nAlignedMem + nOffsetBytes) <= m_anHeap[1] ) {
		m_anHeap[0] = nAlignedMem + nOffsetBytes;
		return (void *)(nAlignedMem - nOffset);
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocHeap0(with offset) - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocAndZeroHeap0( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	u32 nOffsetHeap, nOffsetBytes, nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );
	FASSERT( fmath_IsPowerOf2( nOffset, TRUE ) );
	FASSERT( nOffset <= nBytes );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nOffsetBytes = nBytes - nOffset;
	nOffsetHeap = m_anHeap[0] + nOffset;
	nAlignedMem = FMATH_BYTE_ALIGN_UP( nOffsetHeap, nByteAlignment );

	if( (nAlignedMem + nOffsetBytes) <= m_anHeap[1] ) {
		m_anHeap[0] = nAlignedMem + nOffsetBytes;
		nAlignedMem -= nOffset;
		fang_MemZero( (void *)nAlignedMem, nBytes );
		return (void *)nAlignedMem;
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocAndZeroHeap0(with offset) - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocHeap1( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	u32 nOffsetBytes, nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );
	FASSERT( fmath_IsPowerOf2( nOffset, TRUE ) );
	FASSERT( nOffset <= nBytes );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nOffsetBytes = m_anHeap[1] - (nBytes - nOffset);
	nAlignedMem = FMATH_BYTE_ALIGN_DOWN( nOffsetBytes, nByteAlignment ) - nOffset;

	if( nAlignedMem >= m_anHeap[0] ) {
		m_anHeap[1] = nAlignedMem;
		return (void *)nAlignedMem;
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocHeap1(with offset) - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocAndZeroHeap1( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	u32 nOffsetBytes, nAlignedMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );
	FASSERT( fmath_IsPowerOf2( nOffset, TRUE ) );
	FASSERT( nOffset <= nBytes );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nOffsetBytes = m_anHeap[1] - (nBytes - nOffset);
	nAlignedMem = FMATH_BYTE_ALIGN_DOWN( nOffsetBytes, nByteAlignment ) - nOffset;

	if( nAlignedMem >= m_anHeap[0] ) {
		m_anHeap[1] = nAlignedMem;
		fang_MemZero( (void *)nAlignedMem, nBytes );
		return (void *)nAlignedMem;
	}

#if FANG_PLATFORM_GC
	OSReport( "CFHeap::AllocAndZeroHeap1(with offset) - REQUEST FOR MEMORY ALLOCATION OF %d BYTES FAILED!!!\n", nBytes );
#endif
	return NULL;
}

void *CFHeap::AllocAllHeap0( u32 nByteAlignment, u32 *pnBytesAllocated ) {
	u32 nAlignedMem, nBytesToAlloc;
	void *pMem;

	FASSERT( m_nHeapBytes );
	FASSERT( fmath_IsPowerOf2( nByteAlignment, TRUE ) );

	if( nByteAlignment == 0 ) {
		nByteAlignment = sizeof(u32);
	}

	nAlignedMem = FMATH_BYTE_ALIGN_UP( m_anHeap[0], nByteAlignment );
	if( nAlignedMem >= m_anHeap[1] ) {
		return NULL;
	}

	nBytesToAlloc = m_anHeap[1] - nAlignedMem;

	pMem = Alloc( 0, nBytesToAlloc, nByteAlignment );
	FASSERT( pMem != NULL );

	if( pnBytesAllocated ) {
		*pnBytesAllocated = nBytesToAlloc;
	}

	return pMem;
}

BOOL CFHeap::ResizeHeap0( void *pMemBlock, u32 nNewSizeInBytes ) {
	u32 nMemBlock;

	FASSERT( m_nHeapBytes );

	if( pMemBlock == NULL ) {
		return TRUE;
	}

	nMemBlock = (u32)pMemBlock;

	FASSERT( nMemBlock >= m_anHeapExtents[0] );
	FASSERT( nMemBlock < m_anHeapExtents[1] );
	FASSERT( nMemBlock <= m_anHeap[0] );

	if( (nMemBlock + nNewSizeInBytes) <= m_anHeap[1] ) {
		m_anHeap[0] = nMemBlock + nNewSizeInBytes;
		return TRUE;
	}

	return FALSE;
}

BOOL CFHeap::ResizeHeap1ToNewBase( void *pOldBase, void *pNewBase ) {
#if FANG_PLATFORM_GC
	#pragma unused( pOldBase )
#endif

	u32 nMemBlock;

	FASSERT( m_nHeapBytes );

	if( pNewBase == NULL ) {
		return TRUE;
	}

	nMemBlock = (u32)pNewBase;

	FASSERT( nMemBlock >= m_anHeapExtents[0] );
	FASSERT( nMemBlock < m_anHeapExtents[1] );
	FASSERT( (u32)pOldBase == m_anHeap[1] );

	if( nMemBlock >= m_anHeap[0] ) {
		m_anHeap[1] = nMemBlock;
		return TRUE;
	}

	return FALSE;
}

