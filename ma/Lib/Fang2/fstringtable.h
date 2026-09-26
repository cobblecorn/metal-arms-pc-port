//////////////////////////////////////////////////////////////////////////////////////
// fstringtable.h - Fang string table class.
//
// Author: Steve Ranck     
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
// 04/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FSTRINGTABLE_H_
#define _FSTRINGTABLE_H_ 1

#include "fang.h"
#include "flinklist.h"



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFStringCluster
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CFStringCluster {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FLink_t m_Link;							// Link to other clusters in this table
	u32 m_nMaxStringCount;					// Maximum number of strings this cluster can hold
	u32 m_nStringCount;						// The number of strings that have been added to this cluster so far
	void **m_ppStringArray;					// Array of strings in this cluster (can be chars or wchars)
	u8 *m_paStringBitTypes;					// Array of string bit ID's... if bit is 1, then string is a wide string



	
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CFStringCluster();

	static CFStringCluster *CreateCluster( void );

	FINLINE u32 GetFreeElementCount( void ) const { return m_nMaxStringCount - m_nStringCount; }

	//Single byte string interfaces
	cchar *FindString( cchar *pszString ) const;
	cchar *AddString( cchar *pszString );
	s32 ComputeStringIndex( cchar *pszString ) const;

	//WideChar Interfaces
	cwchar *FindString( cwchar *pwszString ) const;
	cwchar *AddString( cwchar *pwszString );
	s32 ComputeStringIndex( cwchar *pwszString ) const;


	friend class CFStringTable;
	FCLASS_STACKMEM_NOALIGN( CFStringCluster );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFStringTable
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CFStringTable {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FLink_t m_Link;							// Link to other string tables
	cchar *m_pszTableName;					// Pointer to table name
	FLinkRoot_t m_ClusterLinkRoot;			// Link list of clusters
	CFStringCluster *m_pLastCluster;		// Pointer to the last cluster we added to our table (NULL=none)
	u32 m_nStringCount;						// Total strings in this table

	static BOOL m_bModuleInitialized;		// TRUE when system is initialized
	static FLinkRoot_t m_TableRoot;			// Linklist of string tables




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static void ResFrameReleased( void *pReleasedMem );

	static CFStringTable *FindTable( cchar *pszTableName );
	static CFStringTable *CreateTable( cchar *pszTableName, BOOL bErrorIfAlreadyExists=FALSE );

	static cchar *FindString( cchar *pszTableName, cchar *pszString );
	static cchar *AddString( cchar *pszTableName, cchar *pszString, BOOL bAutoCreateTable=TRUE, BOOL bErrorIfAlreadyExists=FALSE );

	static cwchar *FindString( cchar *pszTableName, cwchar *pwszString );
	static cwchar *AddString( cchar *pszTableName, cwchar *pwszString, BOOL bAutoCreateTable=TRUE, BOOL bErrorIfAlreadyExists=FALSE );

	FINLINE u32 GetStringCount( void ) const { return m_nStringCount; }

	s32 ComputeStringIndex( cchar *pszString ) const;
	cchar *FindString( cchar *pszString ) const;
	cchar *AddString( cchar *pszString, BOOL bErrorIfAlreadyExists=FALSE );

	s32 ComputeStringIndex( cwchar *pwszString ) const;
	cwchar *FindString( cwchar *pwszString ) const;
	cwchar *AddString( cwchar *pwszString, BOOL bErrorIfAlreadyExists=FALSE );



//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CFStringTable();

	CFStringCluster *AddNewCluster( void );

	static FINLINE CFStringTable *GetFirstTable( void ) { FASSERT(m_bModuleInitialized); return (CFStringTable *)flinklist_GetHead( &m_TableRoot ); }
	static FINLINE CFStringTable *GetLastTable( void ) { FASSERT(m_bModuleInitialized); return (CFStringTable *)flinklist_GetTail( &m_TableRoot ); }
	FINLINE CFStringTable *GetNextTable( void ) const { return (CFStringTable *)flinklist_GetNext( &m_TableRoot, this ); }
	FINLINE CFStringTable *GetPrevTable( void ) const { return (CFStringTable *)flinklist_GetPrev( &m_TableRoot, this ); }
	FINLINE void AddTable( void ) { flinklist_AddTail( &m_TableRoot, this ); }
	static FINLINE void RemoveLastTable( void ) { FASSERT(m_bModuleInitialized); flinklist_RemoveTail( &m_TableRoot ); }

	FINLINE CFStringCluster *GetFirstCluster( void ) const { return (CFStringCluster *)flinklist_GetHead( &m_ClusterLinkRoot ); }
	FINLINE CFStringCluster *GetLastCluster( void ) const { return (CFStringCluster *)flinklist_GetTail( &m_ClusterLinkRoot ); }
	FINLINE CFStringCluster *GetNextCluster( const CFStringCluster *pCluster ) const { return (CFStringCluster *)flinklist_GetNext( &m_ClusterLinkRoot, pCluster ); }
	FINLINE CFStringCluster *GetPrevCluster( const CFStringCluster *pCluster ) const { return (CFStringCluster *)flinklist_GetPrev( &m_ClusterLinkRoot, pCluster ); }
	FINLINE void AddCluster( CFStringCluster *pCluster ) { flinklist_AddTail( &m_ClusterLinkRoot, pCluster ); }
	FINLINE void RemoveLastCluster( void ) { flinklist_RemoveTail( &m_ClusterLinkRoot ); }


	FCLASS_STACKMEM_NOALIGN( CFStringTable );
} FCLASS_NOALIGN_SUFFIX;




#endif

