//////////////////////////////////////////////////////////////////////////////////////
// FCheckPoint.h - engine/game load and save functionality.
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
#ifndef _FCHECKPOINT_H_
#define _FCHECKPOINT_H_ 1

#include "fang.h"
#include "fres.h"
#include "ftext.h"
#include "fstorage.h"
#include "fAMem.h"

#define FCHECKPOINT_MAX_OBJECTS		( 1024 )	// max handles to data available per checkpoint.
#define FCHECKPOINT_MAX_CHECKPOINTS	( 2 )		// max checkpoints stored in memory. number should be even, for data alignment

// state of a checkpoint
typedef enum
{
	FCHECKPOINT_NULL_STATE,
	FCHECKPOINT_LOAD_STATE,
	FCHECKPOINT_SAVE_STATE,
} FCheckPointState_t;

// info needed to save and load one object's data
typedef struct
{
	u32		nStreamOffset;		// offset into storage stream where this object's data is stored (always multiples of m_pMemBuffer size)
	u32		nDataSize;			// size of data stored for this object
	BOOL	bInUse;				// TRUE if this object data handle is in use
} FCheckPointObjectData_t;


// data describing a checkpoint
typedef struct
{
	CFAMemAccessor			*m_pMemAccessor;					// pointer to storage access object
	void					*m_pMemBuffer;						// temporary buffer to store pending reads/writes to storage
	u32						m_nMemBufferOffset;					// current offset into m_pMemBuffer
	u32						m_nStreamSize;						// storage size of stream in bytes
	u32						m_nStreamOffset;					// current offset into stream (should always be multiples of storage alignment size)
	s32						m_hCurObj;							// current object data handle
	FCheckPointObjectData_t	*m_aObjData;						// pointer to an array of object data structs, indexed by object data handle
	FCheckPointState_t		m_nState;							// state of CheckPoint system
} FCheckPointData_t;


FCLASS_ALIGN_PREFIX class CFCheckPoint
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef s32 ObjectDataHandle_t;
	typedef s32 CheckPointHandle_t;

	enum
	{
		NULL_HANDLE = -1,
	};

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL					m_bModuleInitialized;
	static s32					m_nCurCP;								// index of current check point
	static FCheckPointData_t	m_aPoint[ FCHECKPOINT_MAX_CHECKPOINTS ];	// array of checkpoints

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static BOOL SetCheckPoint( s32 nCheckpoint );
	static s32 GetCheckPoint( void );
	static BOOL SetSaveState( void );
	static BOOL DoneSaving( void );
	static BOOL SetLoadState( void );
	static BOOL DoneLoading( void );
	static ObjectDataHandle_t CreateObjectDataHandle( void );
	static void DestroyObjectDataHandle( ObjectDataHandle_t hHandle );
	static BOOL SetObjectDataHandle( ObjectDataHandle_t hHandle );

	static BOOL SaveString( cchar *pszString );
	static BOOL SaveData( void *pData, u32 size );
	static BOOL SaveData( const u8 &rData );
	static BOOL SaveData( const u16 &rData );
	static BOOL SaveData( const u32 &rData );
	static BOOL SaveData( const u64 &rData );
	static BOOL SaveData( const s8 &rData );
	static BOOL SaveData( const s16 &rData );
	static BOOL SaveData( const s32 &rData );
	static BOOL SaveData( const f32 &rData );
#if FANG_PLATFORM_GC
	static BOOL SaveData( const BOOL &rData );	// for BOOL type on windows platform
#endif
	static BOOL SaveData( const CFVec3A &rData );
	static BOOL SaveData( const CFQuatA &rData );
	static BOOL SaveData( const CFMtx43A &rData );

	static BOOL LoadData( void *pData, u32 size );
	static u32 LoadStringSize( void );
	static BOOL LoadStringData( char *pszString, u32 nSize );
	static BOOL LoadData( u8 &rData );
	static BOOL LoadData( u16 &rData );
	static BOOL LoadData( u32 &rData );
	static BOOL LoadData( u64 &rData );
	static BOOL LoadData( s8 &rData );
	static BOOL LoadData( s16 &rData );
	static BOOL LoadData( s32 &rData );
	static BOOL LoadData( f32 &rData );
#if FANG_PLATFORM_GC
	static BOOL LoadData( BOOL &rData );	// for BOOL type on windows platform
#endif
	static BOOL LoadData( CFVec3A &rData );
	static BOOL LoadData( CFQuatA &rData );
	static BOOL LoadData( CFMtx43A &rData );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _WillStreamOverflow( FCheckPointData_t *pPoint, u32 uSize );
	static BOOL _SaveDataToStream( FCheckPointData_t *pPoint, void* pData, u32 uSize );
	static BOOL _LoadDataFromStream( FCheckPointData_t *pPoint, void *pData, u32 uSize );
	static BOOL _FlushMemBufferToStorage( FCheckPointData_t *pPoint );
	static BOOL _FillMemBufferFromStorage( FCheckPointData_t *pPoint );


	FCLASS_STACKMEM_ALIGN( CFCheckPoint );
} FCLASS_ALIGN_SUFFIX;
// end CFCheckPoint class
//----------------------------------------------------------------------------------------------------------------------------------

#endif

