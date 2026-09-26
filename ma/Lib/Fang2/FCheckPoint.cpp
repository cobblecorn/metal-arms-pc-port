//////////////////////////////////////////////////////////////////////////////////////
// FCheckPoint.cpp - engine/game load and save functionality.
//
// Author: Chris MacDonald
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
// 09/28/02 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "FCheckPoint.h"
#include "fclib.h"

#if FANG_PLATFORM_WIN && FANG_WINGC
// The GameCube's 304 KB was an ARAM budget; retail saves reach ~308 KB (wewhchase01 overflowed).
#define _CHECKPOINT_STREAM_SIZE		( 1024 * 1024 )
#else
#define _CHECKPOINT_STREAM_SIZE		( 1024 * 304 )
#endif
#define _CHECKPOINT_MEM_BUFFER_SIZE	( 1024 )	// size of buffer to hold pending reads/writes to storage
#define _STREAM_ALIGNMENT_BYTES		( 32 )		// alignment boundry requirement of CFMemAccessor
#define _MAX_WRITE_RETRY			( 8 )		// number of time to try to write to storage before failing

// Note: the test heap check doesn't work with the partial restores of
// multiplayer, so if you want to use it for debugging in single player
// enable on your local build.
//#define _CHECK_TEST_HEAP 1
#ifdef _CHECK_TEST_HEAP
static u8 _TestHeap[FCHECKPOINT_MAX_CHECKPOINTS][_CHECKPOINT_STREAM_SIZE];
static u8 _TestBuffer[ 1024 * 8 ];
static u32 _TestHeapOffset = 0;
#endif

//------------------------------------------------------------------------------
// CFCheckPoint static member variables
//------------------------------------------------------------------------------

s32	CFCheckPoint::m_nCurCP;								// handle to current check point
FCheckPointData_t CFCheckPoint::m_aPoint[ FCHECKPOINT_MAX_CHECKPOINTS ];	// array of checkpoints
BOOL CFCheckPoint::m_bModuleInitialized = FALSE;

//------------------------------------------------------------------------------
// CFCheckPoint class functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
BOOL CFCheckPoint::ModuleStartup( void )
{
	FASSERT( !m_bModuleInitialized );

	if(	Fang_ConfigDefs.bCheckPoint_StartupSystem == FALSE ) {
		return TRUE;
	}

	s32 nCheckpoint, nIndex;
//	CFCheckPoint::_FCheckPointData_t *pPoint;
	FCheckPointData_t *pPoint;
	BOOL bResult;
	FResFrame_t hFrame;

	hFrame = fres_GetFrame();
	m_nCurCP = -1;

	for( nCheckpoint = 0; nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS; nCheckpoint++ )
	{
		// loop through and init every checkpoint

		pPoint = &m_aPoint[nCheckpoint];

		pPoint->m_pMemAccessor = fnew CFAMemAccessor;
		FASSERT( pPoint->m_pMemAccessor );
		if( pPoint->m_pMemAccessor == NULL )
		{
			goto _ExitWithError;
		}

		FASSERT( _CHECKPOINT_STREAM_SIZE % _STREAM_ALIGNMENT_BYTES == 0 );
		bResult = pPoint->m_pMemAccessor->Init( _CHECKPOINT_STREAM_SIZE, FAMEM_TYPE_SLOW );
		FASSERT( bResult );
		if( !bResult )
		{
			goto _ExitWithError;
		}

		FASSERT( _CHECKPOINT_MEM_BUFFER_SIZE % _STREAM_ALIGNMENT_BYTES == 0 );
		pPoint->m_pMemBuffer = fres_AlignedAlloc( _CHECKPOINT_MEM_BUFFER_SIZE, _STREAM_ALIGNMENT_BYTES );
		FASSERT( pPoint->m_pMemBuffer );
		if( pPoint->m_pMemBuffer == NULL )
		{
			goto _ExitWithError;
		}
		pPoint->m_nMemBufferOffset = 0;

		pPoint->m_nState = FCHECKPOINT_NULL_STATE;
		pPoint->m_nStreamSize = _CHECKPOINT_STREAM_SIZE;
		pPoint->m_nStreamOffset = 0;
		pPoint->m_hCurObj = NULL_HANDLE;

		pPoint->m_aObjData = (FCheckPointObjectData_t*) fres_Alloc( FCHECKPOINT_MAX_OBJECTS * sizeof( FCheckPointObjectData_t ) );
		FASSERT( pPoint->m_aObjData );
		if( pPoint->m_aObjData == NULL )
		{
			goto _ExitWithError;
		}

		for( nIndex = 0; nIndex < FCHECKPOINT_MAX_OBJECTS; nIndex++ )
		{
			// init all data objects

			pPoint->m_aObjData[nIndex].bInUse = FALSE;
			pPoint->m_aObjData[nIndex].nDataSize = 0;
			pPoint->m_aObjData[nIndex].nStreamOffset = 0;
		}
	}

	m_bModuleInitialized = TRUE;

	return TRUE;

_ExitWithError:
	ModuleShutdown();
	fres_ReleaseFrame( hFrame );
	return FALSE;
}

//------------------------------------------------------------------------------
void CFCheckPoint::ModuleShutdown( void )
{
	for( s32 nCheckpoint = 0; nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS; nCheckpoint++ )
	{
		if( m_aPoint[nCheckpoint].m_pMemAccessor != NULL )
		{
			m_aPoint[nCheckpoint].m_pMemAccessor->Free();
			fdelete( m_aPoint[nCheckpoint].m_pMemAccessor );
			m_aPoint[nCheckpoint].m_pMemAccessor = NULL;
		}
	}
	m_bModuleInitialized = FALSE;
}

//------------------------------------------------------------------------------
// Sets the context of the checkpoint system to a particular checkpoint.
// (Must be called before loading or saving begins for the first time.)
BOOL CFCheckPoint::SetCheckPoint( s32 nCheckpoint )
{
	FASSERT( m_bModuleInitialized );
	FASSERT( nCheckpoint >= 0 && nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS );

	if( nCheckpoint < 0 || nCheckpoint >= FCHECKPOINT_MAX_CHECKPOINTS )
	{
		return FALSE;
	}

	m_nCurCP = nCheckpoint;

	return TRUE;
}

//------------------------------------------------------------------------------
// returns the currently set checkpoint index
s32 CFCheckPoint::GetCheckPoint( void )
{
	FASSERT( m_bModuleInitialized );
	FASSERT( m_nCurCP >= 0 && m_nCurCP < FCHECKPOINT_MAX_CHECKPOINTS && m_nCurCP != NULL_HANDLE );

	if( m_nCurCP < 0 || m_nCurCP >= FCHECKPOINT_MAX_CHECKPOINTS || m_nCurCP == NULL_HANDLE )
	{
		return -1;
	}

	return m_nCurCP;
}

//------------------------------------------------------------------------------
// prepares the system to begin saving data,
// destroys all object data handles.
BOOL CFCheckPoint::SetSaveState( void )
{
	FASSERT( m_bModuleInitialized );
	FASSERT( m_nCurCP != NULL_HANDLE );
	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	pPoint->m_nState = FCHECKPOINT_SAVE_STATE;
	pPoint->m_nStreamOffset = 0;
	pPoint->m_hCurObj = NULL_HANDLE;
	pPoint->m_nMemBufferOffset = 0;

	// reset object data array
	for( int nIndex = 0; nIndex < FCHECKPOINT_MAX_OBJECTS; nIndex++ )
	{
		pPoint->m_aObjData[ nIndex ].bInUse = FALSE;
		pPoint->m_aObjData[ nIndex ].nDataSize = 0;
		pPoint->m_aObjData[ nIndex ].nStreamOffset = 0;
	}

#ifdef _CHECK_TEST_HEAP
_TestHeapOffset = 0;
#endif

	return TRUE;
}

//------------------------------------------------------------------------------
// This function must be called when all saving
// is finished.  Flushes mem buffer to storage
BOOL CFCheckPoint::DoneSaving( void )
{
	FASSERT( m_bModuleInitialized );
	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	FASSERT( pPoint->m_nState == FCHECKPOINT_SAVE_STATE );

	if( pPoint->m_hCurObj != NULL_HANDLE )
	{
		// there is a currently active data object.
		// flush mem buffer to storage
		_FlushMemBufferToStorage( pPoint );
	}

	DEVPRINTF( "Checkpoint save used %d bytes.\n", pPoint->m_nStreamOffset );
	return TRUE;
}

//------------------------------------------------------------------------------
// prepares the system to begin loadng data,
BOOL CFCheckPoint::SetLoadState( void )
{
	FASSERT( m_bModuleInitialized );
	m_aPoint[m_nCurCP].m_nState = FCHECKPOINT_LOAD_STATE;

#ifdef _CHECK_TEST_HEAP
_TestHeapOffset = 0;
#endif

	return TRUE;
}

//------------------------------------------------------------------------------
// This function must be called when all loading
// is finished.
BOOL CFCheckPoint::DoneLoading( void )
{
	FASSERT( m_bModuleInitialized );
	return TRUE;
}


//------------------------------------------------------------------------------
// Create a handle to a set of save data for an object.
// Must be called before calls to SaveData() are made.
// It is not legal to create an object data handle in the load state.
CFCheckPoint::ObjectDataHandle_t CFCheckPoint::CreateObjectDataHandle( void )
{
	FASSERT( m_bModuleInitialized );
	FASSERT( m_nCurCP != NULL_HANDLE );
	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	FASSERT( pPoint->m_nState == FCHECKPOINT_SAVE_STATE );

	if( pPoint->m_hCurObj != NULL_HANDLE )
	{
		// there is a currently active data object.
		// flush mem buffer to storage before moving on to next object.
		_FlushMemBufferToStorage( pPoint );
	}

	for( int nIndex = 0; nIndex < FCHECKPOINT_MAX_OBJECTS; nIndex++ )
	{
		if( pPoint->m_aObjData[ nIndex ].bInUse == FALSE )
		{
			FASSERT( pPoint->m_nStreamOffset % _STREAM_ALIGNMENT_BYTES == 0 );
			pPoint->m_aObjData[ nIndex ].bInUse = TRUE;
			pPoint->m_aObjData[ nIndex ].nDataSize = 0;
			pPoint->m_aObjData[ nIndex ].nStreamOffset = pPoint->m_nStreamOffset;
			pPoint->m_hCurObj = nIndex;
			return nIndex;
		}
	}

	// not enough handles, increase FCHECKPOINT_MAX_OBJECTS
	FASSERT_NOW;
	return NULL_HANDLE;
}

//------------------------------------------------------------------------------
// In the load state, tells the CheckPoint system which data object to load data from.
BOOL CFCheckPoint::SetObjectDataHandle( ObjectDataHandle_t hHandle )
{
	FASSERT( m_bModuleInitialized );
	FASSERT( m_nCurCP != NULL_HANDLE );
	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	FASSERT( pPoint->m_nState == FCHECKPOINT_LOAD_STATE );
	FASSERT( hHandle >= 0 && hHandle < FCHECKPOINT_MAX_OBJECTS && hHandle != NULL_HANDLE );

	if( hHandle >= 0 && hHandle < FCHECKPOINT_MAX_OBJECTS && hHandle != NULL_HANDLE )
	{
		pPoint->m_hCurObj = hHandle;
		pPoint->m_nStreamOffset = pPoint->m_aObjData[ hHandle ].nStreamOffset;
		FASSERT( pPoint->m_nStreamOffset % _STREAM_ALIGNMENT_BYTES == 0 );

		// fill mem buffer for first LoadData calls
		_FillMemBufferFromStorage( pPoint );
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
// CFCheckPoint class save functions
//------------------------------------------------------------------------------

#define _SAVE_DATA_CHECK()\
{\
	FASSERT( m_nCurCP >= 0 && m_nCurCP < FCHECKPOINT_MAX_CHECKPOINTS );\
	FASSERT( m_aPoint[m_nCurCP].m_nState == FCHECKPOINT_SAVE_STATE );\
	FASSERT( m_aPoint[m_nCurCP].m_hCurObj != NULL_HANDLE );\
	FASSERT( m_aPoint[m_nCurCP].m_aObjData[ m_aPoint[m_nCurCP].m_hCurObj ].bInUse );\
	if( m_aPoint[m_nCurCP].m_nState != FCHECKPOINT_SAVE_STATE )\
	{\
		return FALSE;\
	}\
	if( m_aPoint[m_nCurCP].m_hCurObj == NULL_HANDLE )\
	{\
		return FALSE;\
	}\
	if( !m_aPoint[m_nCurCP].m_aObjData[ m_aPoint[m_nCurCP].m_hCurObj ].bInUse )\
	{\
		return FALSE;\
	}\
}\


//------------------------------------------------------------------------------
// returns TRUE if adding nSize to stream will cause overflow
BOOL CFCheckPoint::_WillStreamOverflow( FCheckPointData_t *pPoint, u32 uSize )
{
	FASSERT( m_bModuleInitialized );
	FASSERT( pPoint->m_nStreamOffset + pPoint->m_nMemBufferOffset + uSize < pPoint->m_nStreamSize );
	if( pPoint->m_nStreamOffset + pPoint->m_nMemBufferOffset + uSize >= pPoint->m_nStreamSize )
	{
		return TRUE;
	}
	
	return FALSE;
}

//------------------------------------------------------------------------------
// copies mem buffer to storage, returns TRUE if successful
BOOL CFCheckPoint::_FlushMemBufferToStorage( FCheckPointData_t *pPoint )
{
	FASSERT( m_bModuleInitialized );
	FAmem_Error_e eError;
	u32 uWriteSize;

	if( pPoint->m_nMemBufferOffset % _STREAM_ALIGNMENT_BYTES )
	{
		uWriteSize = pPoint->m_nMemBufferOffset + ( _STREAM_ALIGNMENT_BYTES - (pPoint->m_nMemBufferOffset % _STREAM_ALIGNMENT_BYTES) );
	}
	else
	{
		uWriteSize = pPoint->m_nMemBufferOffset;
	}

	FASSERT( uWriteSize % _STREAM_ALIGNMENT_BYTES == 0 );

	for( s32 nRetry = 0; nRetry < _MAX_WRITE_RETRY; nRetry++ )
	{
		eError = pPoint->m_pMemAccessor->Write( pPoint->m_nStreamOffset, pPoint->m_pMemBuffer, uWriteSize );
		if( eError == FAMEM_ERROR_NONE )
		{
			break;
		}

		// should wait for a while here?
	}

	FASSERT( eError == FAMEM_ERROR_NONE );

	if( eError == FAMEM_ERROR_NONE )
	{
		// update offsets for stream and mem buffer
		pPoint->m_nMemBufferOffset = 0;
		pPoint->m_nStreamOffset += uWriteSize;
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
//  saves data to the checkpoint storage stream
BOOL CFCheckPoint::_SaveDataToStream( FCheckPointData_t *pPoint, void* pData, u32 uSize )
{
	FASSERT( m_bModuleInitialized );
	u32 uWriteSize;
	u8 *puData = (u8*) pData;
	u32 uRemainingSize = uSize;

	while( pPoint->m_nMemBufferOffset + uRemainingSize > _CHECKPOINT_MEM_BUFFER_SIZE )
	{
		// requested write exceeds remaining space in m_nMemBuffer.

		// fill remaining space in m_nMemBuffer
		uWriteSize = _CHECKPOINT_MEM_BUFFER_SIZE - pPoint->m_nMemBufferOffset;
		fang_MemCopy( (u8*)pPoint->m_pMemBuffer + pPoint->m_nMemBufferOffset, puData, uWriteSize );
		pPoint->m_nMemBufferOffset += uWriteSize;
		puData += uWriteSize;
		uRemainingSize -= uWriteSize;
		FASSERT( uRemainingSize > 0 );

		if( !_FlushMemBufferToStorage( pPoint ) )
		{
			return FALSE;
		}

		// record increase in stored data size
		pPoint->m_aObjData[ pPoint->m_hCurObj ].nDataSize += uWriteSize;
	}

	if( uRemainingSize )
	{
		// write data to m_pMemBuffer
		fang_MemCopy( (u8*)pPoint->m_pMemBuffer + pPoint->m_nMemBufferOffset, puData, uRemainingSize );
		pPoint->m_nMemBufferOffset += uRemainingSize;
	}

	// record increase in stored data size
	pPoint->m_aObjData[ pPoint->m_hCurObj ].nDataSize += uRemainingSize;

#ifdef _CHECK_TEST_HEAP
	fang_MemCopy( &_TestHeap[m_nCurCP][ _TestHeapOffset ], pData, uSize );
	_TestHeapOffset += uSize;
#endif

	return TRUE;
}

//------------------------------------------------------------------------------
// saves raw data chunk pointed to by "pData" and of size "size".
BOOL CFCheckPoint::SaveData( void *pData, u32 uSize )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	FASSERT( uSize != 0 );

	if( uSize % 4 )
	{
		uSize = uSize + ( 4 - ( uSize % 4 ) );
		FASSERT( uSize % 4 == 0 );
	}

	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, pData, uSize );

	return TRUE;
}

//------------------------------------------------------------------------------
// save a string (and its size)
BOOL CFCheckPoint::SaveString( cchar *pszString )
{
	FASSERT( m_bModuleInitialized );
	u32 uSize;
	_SAVE_DATA_CHECK();

	FASSERT( FALSE );

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	if( pszString != NULL )
	{
		uSize = fclib_strlen( pszString ) + 1;
	}
	else
	{
		uSize = 1;
	}

	FASSERT( uSize != 0 );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	// copy size first
	_SaveDataToStream( pPoint, (void*) &uSize, sizeof(uSize) );

	// copy string
	if( pszString != NULL && uSize > 1 )
	{
		_SaveDataToStream( pPoint, (void*) pszString, uSize );
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// save a u8 (with 16 bit size to preserve alignment)
BOOL CFCheckPoint::SaveData( const u8 &rData )
{
	FASSERT( m_bModuleInitialized );
	u32 uTemp = (u32) rData;

	return SaveData( uTemp );

//	return TRUE;
}


//------------------------------------------------------------------------------
// save a u16
BOOL CFCheckPoint::SaveData( const u16 &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	u32 uTemp = (u32) rData;

	return SaveData( uTemp );

//	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];
//
//	u32 uSize = sizeof( rData );
//	if( _WillStreamOverflow( pPoint, uSize ) )
//	{
//		return FALSE;
//	}
//
//	_SaveDataToStream( pPoint, (void*) &rData, uSize );

//	return TRUE;
}

//------------------------------------------------------------------------------
// save a u32
BOOL CFCheckPoint::SaveData( const u32 &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData, uSize );

	return TRUE;
}

//------------------------------------------------------------------------------
// save a u64
BOOL CFCheckPoint::SaveData( const u64 &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData, uSize );

	return TRUE;
}

//------------------------------------------------------------------------------
// save a s8
BOOL CFCheckPoint::SaveData( const s8 &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	s32 nTemp = (s32) rData;

	return SaveData( nTemp );
}

//------------------------------------------------------------------------------
// save a s16
BOOL CFCheckPoint::SaveData( const s16 &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	s32 nTemp = (s32) rData;

	return SaveData( nTemp );
}

//------------------------------------------------------------------------------
// save a s32
BOOL CFCheckPoint::SaveData( const s32 &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData, uSize );

	return TRUE;
}

//------------------------------------------------------------------------------
// save a f32
BOOL CFCheckPoint::SaveData( const f32 &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData, uSize );

	return TRUE;
}

#if FANG_PLATFORM_GC
//------------------------------------------------------------------------------
// save a BOOL
BOOL CFCheckPoint::SaveData( const BOOL &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData, uSize );

	return TRUE;
}
#endif


//------------------------------------------------------------------------------
// save a vector
BOOL CFCheckPoint::SaveData( const CFVec3A &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData.a );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData.a, uSize );

	return TRUE;
}

//------------------------------------------------------------------------------
// save a quaternion
BOOL CFCheckPoint::SaveData( const CFQuatA &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData.a );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData.a, uSize );

	return TRUE;
}

//------------------------------------------------------------------------------
// save a matrix
BOOL CFCheckPoint::SaveData( const CFMtx43A &rData )
{
	FASSERT( m_bModuleInitialized );
	_SAVE_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	u32 uSize = sizeof( rData.a );
	if( _WillStreamOverflow( pPoint, uSize ) )
	{
		return FALSE;
	}

	_SaveDataToStream( pPoint, (void*) &rData.a, uSize );

	return TRUE;
}

//------------------------------------------------------------------------------
// CFCheckPoint class load functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define _LOAD_DATA_CHECK()\
{\
	FASSERT( m_nCurCP >= 0 && m_nCurCP < FCHECKPOINT_MAX_CHECKPOINTS );\
	FASSERT( m_aPoint[m_nCurCP].m_nState == FCHECKPOINT_LOAD_STATE );\
	FASSERT( m_aPoint[m_nCurCP].m_hCurObj != NULL_HANDLE );\
	FASSERT( m_aPoint[m_nCurCP].m_aObjData[ m_aPoint[m_nCurCP].m_hCurObj ].bInUse );\
	if( m_aPoint[m_nCurCP].m_nState != FCHECKPOINT_LOAD_STATE )\
	{\
		return FALSE;\
	}\
	if( m_aPoint[m_nCurCP].m_hCurObj == NULL_HANDLE )\
	{\
		return FALSE;\
	}\
	if( !m_aPoint[m_nCurCP].m_aObjData[ m_aPoint[m_nCurCP].m_hCurObj ].bInUse )\
	{\
		return FALSE;\
	}\
}\

//------------------------------------------------------------------------------
// fills mem buffer with data from storage, returns TRUE if successful
BOOL CFCheckPoint::_FillMemBufferFromStorage( FCheckPointData_t *pPoint )
{
	FASSERT( m_bModuleInitialized );
	FAmem_Error_e eError;

	for( s32 nRetry = 0; nRetry < _MAX_WRITE_RETRY; nRetry++ )
	{
		eError = pPoint->m_pMemAccessor->Read( pPoint->m_nStreamOffset, pPoint->m_pMemBuffer, _CHECKPOINT_MEM_BUFFER_SIZE );
		if( eError == FAMEM_ERROR_NONE )
		{
			break;
		}

		// should wait for a while here?
	}

	FASSERT( eError == FAMEM_ERROR_NONE );

	if( eError == FAMEM_ERROR_NONE )
	{
		// update offsets for stream and mem buffer
		pPoint->m_nMemBufferOffset = 0;
		pPoint->m_nStreamOffset += _CHECKPOINT_MEM_BUFFER_SIZE;
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
//  loads data from the checkpoint storage stream
BOOL CFCheckPoint::_LoadDataFromStream( FCheckPointData_t *pPoint, void *pData, u32 uSize )
{
	FASSERT( m_bModuleInitialized );
	u32 uReadSize;
	u8 *puData = (u8*) pData;
	u32 uRemainingSize = uSize;

	while( pPoint->m_nMemBufferOffset + uRemainingSize > _CHECKPOINT_MEM_BUFFER_SIZE )
	{
		// requested read size exceeds data remaining in m_pMemBuffer

		// copy remaining data in m_pMemBuffer to client
		uReadSize = _CHECKPOINT_MEM_BUFFER_SIZE - pPoint->m_nMemBufferOffset;
		fang_MemCopy( puData, (u8*)pPoint->m_pMemBuffer + pPoint->m_nMemBufferOffset, uReadSize );
		pPoint->m_nMemBufferOffset += uReadSize;
		puData += uReadSize;
		uRemainingSize -=uReadSize;

		if( !_FillMemBufferFromStorage( pPoint ) )
		{
			return FALSE;
		}
	}

	if( uRemainingSize )
	{
		fang_MemCopy( puData, (u8*)pPoint->m_pMemBuffer + pPoint->m_nMemBufferOffset, uRemainingSize );
		pPoint->m_nMemBufferOffset += uRemainingSize;
	}

#ifdef _CHECK_TEST_HEAP
	{
		u8* pSource = &_TestHeap[m_nCurCP][ _TestHeapOffset ];
		u8* pDest = (u8*) pData;
		for( u32 nIndex = 0; nIndex < uSize; nIndex++ )
		{
			if( *pDest != *pSource )
			{
				// memory restored from AMem does not match auxiliary copy in _TestHeap.
				FASSERT_NOW;
			}

			pDest++;
			pSource++;
		}
	}
	_TestHeapOffset += uSize;
#endif

	// compute how much data has been loaded for this object so far, and
	// make sure it is less than amount of data that was saved for this object.
	// In other words, check for data loading overflow by client.
	if( ( (pPoint->m_nStreamOffset - _CHECKPOINT_MEM_BUFFER_SIZE) + pPoint->m_nMemBufferOffset) - 
		pPoint->m_aObjData[ pPoint->m_hCurObj ].nStreamOffset <= 
		pPoint->m_aObjData[ pPoint->m_hCurObj ].nDataSize )
	{
		return TRUE;
	}
	else
	{
		// attempting to load more data than was saved for current object
		FASSERT_NOW;
		return FALSE;
	}
}

//------------------------------------------------------------------------------
// loads raw data chunk 
BOOL CFCheckPoint::LoadData( void *pData, u32 uSize )
{
	FASSERT( m_bModuleInitialized );
	u32 dummy;

	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, pData, uSize );

	if( uSize % 4 )
	{
		uSize = 4 - (uSize % 4 );
		_LoadDataFromStream( pPoint, (void*) &dummy, uSize );
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// must be called before loading a string
u32 CFCheckPoint::LoadStringSize( void )
{
	FASSERT( m_bModuleInitialized );
	u32 nStringSize;
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &nStringSize, sizeof( nStringSize ) );

	return nStringSize - 1;
}

//------------------------------------------------------------------------------
// load a string.  Must call LoadStringSize() first.
BOOL CFCheckPoint::LoadStringData( char *pszString, u32 uSize )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	FASSERT( pszString );

	if( uSize == 0 )
	{
		fclib_strcpy( pszString, "" );
		return TRUE;
	}

	_LoadDataFromStream( pPoint, (void *) pszString, uSize + 1 );

	return TRUE;
}

//------------------------------------------------------------------------------
// load a u8
BOOL CFCheckPoint::LoadData( u8 &rData )
{
	FASSERT( m_bModuleInitialized );
	u32 uTemp;

	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &uTemp, sizeof( uTemp ) );
	rData = (u8) uTemp;

	return TRUE;
}


//------------------------------------------------------------------------------
// load a u16
BOOL CFCheckPoint::LoadData( u16 &rData )
{
	FASSERT( m_bModuleInitialized );
	u32 uTemp;

	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

//	_LoadDataFromStream( pPoint, (void *) &rData, sizeof( rData ) );
	_LoadDataFromStream( pPoint, (void *) &uTemp, sizeof( uTemp ) );
	rData = (u16) uTemp;


	return TRUE;
}

//------------------------------------------------------------------------------
// load a u32
BOOL CFCheckPoint::LoadData( u32 &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData, sizeof( rData ) );

	return TRUE;
}

//------------------------------------------------------------------------------
// load a u64
BOOL CFCheckPoint::LoadData( u64 &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData, sizeof( rData ) );

	return TRUE;
}

//------------------------------------------------------------------------------
// load a s8
BOOL CFCheckPoint::LoadData( s8 &rData )
{
	FASSERT( m_bModuleInitialized );
	s32 nTemp;
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &nTemp, sizeof( nTemp ) );
	rData = (s8) nTemp;

	return TRUE;
}

//------------------------------------------------------------------------------
// load a s16
BOOL CFCheckPoint::LoadData( s16 &rData )
{
	FASSERT( m_bModuleInitialized );
	s32 nTemp;
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

//	_LoadDataFromStream( pPoint, (void *) &rData, sizeof( rData ) );
	_LoadDataFromStream( pPoint, (void *) &nTemp, sizeof( nTemp ) );
	rData = (s16) nTemp;

	return TRUE;
}

//------------------------------------------------------------------------------
// load a s32
BOOL CFCheckPoint::LoadData( s32 &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData, sizeof( rData ) );

	return TRUE;
}

//------------------------------------------------------------------------------
// load a f32
BOOL CFCheckPoint::LoadData( f32 &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData, sizeof( rData ) );

	return TRUE;
}

#if FANG_PLATFORM_GC
//------------------------------------------------------------------------------
// load a BOOL
BOOL CFCheckPoint::LoadData( BOOL &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData, sizeof( rData ) );

	return TRUE;
}
#endif

//------------------------------------------------------------------------------
// load a vector
BOOL CFCheckPoint::LoadData( CFVec3A &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData.a, sizeof( rData.a ) );

	return TRUE;
}

//------------------------------------------------------------------------------
// load a quaternion
BOOL CFCheckPoint::LoadData( CFQuatA &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData.a, sizeof( rData.a ) );

	return TRUE;
}

//------------------------------------------------------------------------------
// load a matrix
BOOL CFCheckPoint::LoadData( CFMtx43A &rData )
{
	FASSERT( m_bModuleInitialized );
	_LOAD_DATA_CHECK();

	FCheckPointData_t *pPoint = &m_aPoint[m_nCurCP];

	_LoadDataFromStream( pPoint, (void *) &rData.a, sizeof( rData.a ) );

	return TRUE;
}

