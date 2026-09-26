//////////////////////////////////////////////////////////////////////////////////////
// fstringtable.cpp - Fang string table class.
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

#include "fang.h"
#include "fstringtable.h"
#include "flinklist.h"
#include "fclib.h"
#include "fres.h"
#include <string.h>

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFStringTable
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CFStringTable::m_bModuleInitialized;
FLinkRoot_t CFStringTable::m_TableRoot;


BOOL CFStringTable::ModuleStartup( void ) {
	FASSERT( !m_bModuleInitialized );

	if( Fang_ConfigDefs.nStringTable_DefaultMaxStringCountPerCluster < 1 ) {
		DEVPRINTF( "CFStringTable::ModuleStartup(): Fang_ConfigDefs.nStringTable_DefaultMaxStringCountPerCluster must be at least 1.\n" );
		return FALSE;
	}

	flinklist_InitRoot( &m_TableRoot, FANG_OFFSETOF( CFStringTable, m_Link ) );

	m_bModuleInitialized = TRUE;

	return TRUE;
}


void CFStringTable::ResFrameReleased( void *pReleasedMem ) {
	CFStringTable *pTable;
	CFStringCluster *pCluster;
	void *pszString;
	u32 i;

	if( m_bModuleInitialized ) {
		// Remove all tables that are wholy destroyed by the frame release...
		while( (pTable = GetLastTable()) && ((void *)pTable < pReleasedMem) ) {
			RemoveLastTable();
		}

		for( pTable=GetFirstTable(); pTable; pTable=pTable->GetNextTable() ) {
			// Remove all clusters that are wholy destroyed by the frame release...
			while( (pCluster = pTable->GetLastCluster()) && ((void *)pCluster < pReleasedMem) ) {
				FASSERT( pTable->m_nStringCount >= pCluster->m_nStringCount );
				pTable->m_nStringCount -= pCluster->m_nStringCount;
				pCluster->m_nStringCount = 0;
				pTable->RemoveLastCluster();
			}

			pCluster = pTable->m_pLastCluster = pTable->GetLastCluster();

			if( pCluster ) {
				for( i=0; i<pCluster->m_nStringCount; i++ ) {
					pszString = pCluster->m_ppStringArray[i];

					if( pszString < pReleasedMem ) {
						// Every string in this cluster from the current string on is to be freed...
						FASSERT( pTable->m_nStringCount >= (pCluster->m_nStringCount - i) );
						pTable->m_nStringCount -= (pCluster->m_nStringCount - i);
						pCluster->m_nStringCount = i;
						break;
					}
				}
			}
		}
	}
}


void CFStringTable::ModuleShutdown( void ) {
	if( m_bModuleInitialized ) {
		m_bModuleInitialized = FALSE;
	}
}


CFStringTable::CFStringTable() {
	m_pszTableName = NULL;
	m_pLastCluster = NULL;
	m_nStringCount = 0;
	flinklist_InitRoot( &m_ClusterLinkRoot, FANG_OFFSETOF( CFStringCluster, m_Link ) );
}


CFStringCluster *CFStringTable::AddNewCluster( void ) {
	CFStringCluster *pCluster;

	pCluster = CFStringCluster::CreateCluster();
	if( pCluster == NULL ) {
		return NULL;
	}

	AddCluster( pCluster );

	m_pLastCluster = pCluster;

	return pCluster;
}


// If bErrorIfAlreadyExists is TRUE, this function will return NULL if the table name already exists.
// If bErrorIfAlreadyExists is FALSE, this function will return the pointer to the existing table of the same name, if one exists.
CFStringTable *CFStringTable::CreateTable( cchar *pszTableName, BOOL bErrorIfAlreadyExists ) {
	CFStringTable *pExistingTable, *pTable;

	FASSERT( m_bModuleInitialized );

	if( pszTableName == NULL ) {
		return NULL;
	}

	FResFrame_t ResFrame = fres_GetFrame();
	pTable = NULL;

	pExistingTable = FindTable( pszTableName );
	if( pExistingTable ) {
		if( bErrorIfAlreadyExists ) {
			DEVPRINTF( "CFStringTable::CreateTable(): String table '%s' already exists.\n", pszTableName );
			goto _ExitWithError;
		} else {
			return pExistingTable;
		}
	}

	pTable = fnew CFStringTable;
	if( pTable == NULL ) {
		DEVPRINTF( "CFStringTable::CreateTable(): Not enough memory to create string table.\n" );
		goto _ExitWithError;
	}

	pTable->m_pszTableName = (cchar *)fres_Alloc( fclib_strlen( pszTableName ) + 1 );
	if( pTable->m_pszTableName == NULL ) {
		DEVPRINTF( "CFStringTable::CreateTable(): Not enough memory to create string table.\n" );
		goto _ExitWithError;
	}
	fclib_strcpy( (char *)pTable->m_pszTableName, pszTableName );

	pTable->AddTable();

	return pTable;

	// Error:
_ExitWithError:
	fdelete( pTable );
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


CFStringTable *CFStringTable::FindTable( cchar *pszTableName ) {
	CFStringTable *pTable;

	FASSERT( m_bModuleInitialized );

	if( pszTableName == NULL ) {
		return NULL;
	}

	if( Fang_ConfigDefs.bStringTable_CaseSensativeTableNames ) {
		for( pTable=GetFirstTable(); pTable; pTable=pTable->GetNextTable() ) {
			if( !fclib_strcmp( pszTableName, pTable->m_pszTableName ) ) {
				return pTable;
			}
		}
	} else {
		for( pTable=GetFirstTable(); pTable; pTable=pTable->GetNextTable() ) {
			if( !fclib_stricmp( pszTableName, pTable->m_pszTableName ) ) {
				return pTable;
			}
		}
	}

	return NULL;
}


cchar *CFStringTable::FindString( cchar *pszTableName, cchar *pszStringName ) {
	CFStringTable *pTable;

	FASSERT( m_bModuleInitialized );

	if( pszStringName == NULL ) {
		return NULL;
	}

	if( pszTableName == NULL ) {
		pszTableName = "Main";
	}

	pTable = FindTable( pszTableName );

	if( pTable == NULL ) {
		return NULL;
	}

	return pTable->FindString( pszStringName );
}


cwchar *CFStringTable::FindString( cchar *pszTableName, cwchar *pwszStringName ) {
	CFStringTable *pTable;

	FASSERT( m_bModuleInitialized );

	if( pwszStringName == NULL ) {
		return NULL;
	}

	if( pszTableName == NULL ) {
		pszTableName = "Main";
	}

	pTable = FindTable( pszTableName );

	if( pTable == NULL ) {
		return NULL;
	}

	return pTable->FindString( pwszStringName );
}


cchar *CFStringTable::FindString( cchar *pszString ) const {
	const CFStringCluster *pCluster;
	cchar *pszClusterString;

	if( pszString == NULL ) {
		return NULL;
	}

	for( pCluster=GetFirstCluster(); pCluster; pCluster=GetNextCluster(pCluster) ) {
		pszClusterString = pCluster->FindString( pszString );

		if( pszClusterString ) {
			return pszClusterString;
		}
	}

	return NULL;
}


cwchar *CFStringTable::FindString( cwchar *pwszString ) const {
	const CFStringCluster *pCluster;
	cwchar *pwszClusterString;

	if( pwszString == NULL ) {
		return NULL;
	}

	for( pCluster=GetFirstCluster(); pCluster; pCluster=GetNextCluster(pCluster) ) {
		pwszClusterString = pCluster->FindString( pwszString );

		if( pwszClusterString ) {
			return pwszClusterString;
		}
	}

	return NULL;
}


s32 CFStringTable::ComputeStringIndex( cchar *pszString ) const {
	const CFStringCluster *pCluster;
	s32 nStringBase, nStringIndex;

	if( pszString == NULL ) {
		return -1;
	}

	nStringBase = 0;

	for( pCluster=GetFirstCluster(); pCluster; pCluster=GetNextCluster(pCluster) ) {
		nStringIndex = pCluster->ComputeStringIndex( pszString );

		if( nStringIndex >= 0 ) {
			// String found...
			return nStringBase + nStringIndex;
		}

		nStringBase += (s32)pCluster->m_nStringCount;
	}

	return -1;
}


s32 CFStringTable::ComputeStringIndex( cwchar *pwszString ) const {
	const CFStringCluster *pCluster;
	s32 nStringBase, nStringIndex;

	if( pwszString == NULL ) {
		return -1;
	}

	nStringBase = 0;

	for( pCluster=GetFirstCluster(); pCluster; pCluster=GetNextCluster(pCluster) ) {
		nStringIndex = pCluster->ComputeStringIndex( pwszString );

		if( nStringIndex >= 0 ) {
			// String found...
			return nStringBase + nStringIndex;
		}

		nStringBase += (s32)pCluster->m_nStringCount;
	}

	return -1;
}


// If bAutoCreateTable is TRUE and the table doesn't exist, a new table is created.
// If bAutoCreateTable is FALSE and the table doesn't exist, NULL is returned.
//
// If bErrorIfAlreadyExists is TRUE, this function will return NULL if the string name already exists in this table.
// If bErrorIfAlreadyExists is FALSE, this function will return the pointer to the existing string of the same name, if one exists in this table.
cchar *CFStringTable::AddString( cchar *pszTableName, cchar *pszString, BOOL bAutoCreateTable, BOOL bErrorIfAlreadyExists ) {
	CFStringTable *pTable;

	if( pszString == NULL ) {
		return NULL;
	}

	if( pszTableName == NULL ) {
		pszTableName = "Main";
	}

	pTable = FindTable( pszTableName );

	if( pTable == NULL ) {
		// Table does not exist...

		if( bAutoCreateTable ) {
			// Create the table...

			pTable = CreateTable( pszTableName, FALSE );
			if( pTable == NULL ) {
				// Could not create the table...
				return NULL;
			}
		} else {
			// Error...
			DEVPRINTF( "CFStringTable::AddString(): Table '%s' already exists.\n", pszTableName );
			return NULL;
		}
	}

	return pTable->AddString( pszString, bErrorIfAlreadyExists );
}


// If bAutoCreateTable is TRUE and the table doesn't exist, a new table is created.
// If bAutoCreateTable is FALSE and the table doesn't exist, NULL is returned.
//
// If bErrorIfAlreadyExists is TRUE, this function will return NULL if the string name already exists in this table.
// If bErrorIfAlreadyExists is FALSE, this function will return the pointer to the existing string of the same name, if one exists in this table.
cwchar *CFStringTable::AddString( cchar *pszTableName, cwchar *pwszString, BOOL bAutoCreateTable, BOOL bErrorIfAlreadyExists ) {
	CFStringTable *pTable;

	if( pwszString == NULL ) {
		return NULL;
	}

	if( pszTableName == NULL ) {
		pszTableName = "Main";
	}

	pTable = FindTable( pszTableName );

	if( pTable == NULL ) {
		// Table does not exist...

		if( bAutoCreateTable ) {
			// Create the table...

			pTable = CreateTable( pszTableName, FALSE );
			if( pTable == NULL ) {
				// Could not create the table...
				return NULL;
			}
		} else {
			// Error...
			DEVPRINTF( "CFStringTable::AddString(): Table '%s' already exists.\n", pszTableName );
			return NULL;
		}
	}

	return pTable->AddString( pwszString, bErrorIfAlreadyExists );
}


// If bErrorIfAlreadyExists is TRUE, this function will return NULL if the string name already exists in this table.
// If bErrorIfAlreadyExists is FALSE, this function will return the pointer to the existing string of the same name, if one exists in this table.
cchar *CFStringTable::AddString( cchar *pszString, BOOL bErrorIfAlreadyExists ) {
	CFStringCluster *pCluster;
	cchar *pszAddedString;

	if( pszString == NULL ) {
		return NULL;
	}

	pszAddedString = FindString( pszString );
	if( pszAddedString ) {
		// String already exists...

		if( !bErrorIfAlreadyExists ) {
			return pszAddedString;
		} else {
			return NULL;
		}
	}

	pCluster = m_pLastCluster;

	if( pCluster==NULL || pCluster->GetFreeElementCount() == 0 ) {
		// No clusters, or no more room in last cluster. Build a new cluster to hold the string...

		pCluster = AddNewCluster();
		if( pCluster == NULL ) {
			return NULL;
		}

		FASSERT( pCluster->GetFreeElementCount() > 0 );

		m_pLastCluster = pCluster;
	}

	pszAddedString = pCluster->AddString( pszString );

	if( pszAddedString ) {
		m_nStringCount++;
	}

	return pszAddedString;
}


// If bErrorIfAlreadyExists is TRUE, this function will return NULL if the string name already exists in this table.
// If bErrorIfAlreadyExists is FALSE, this function will return the pointer to the existing string of the same name, if one exists in this table.
cwchar *CFStringTable::AddString( cwchar *pwszString, BOOL bErrorIfAlreadyExists ) {
	CFStringCluster *pCluster;
	cwchar *pwszAddedString;

	if( pwszString == NULL ) {
		return NULL;
	}

	pwszAddedString = FindString( pwszString );
	if( pwszAddedString ) {
		// String already exists...

		if( !bErrorIfAlreadyExists ) {
			return pwszAddedString;
		} else {
			return NULL;
		}
	}

	pCluster = m_pLastCluster;

	if( pCluster==NULL || pCluster->GetFreeElementCount() == 0 ) {
		// No clusters, or no more room in last cluster. Build a new cluster to hold the string...

		pCluster = AddNewCluster();
		if( pCluster == NULL ) {
			return NULL;
		}

		FASSERT( pCluster->GetFreeElementCount() > 0 );

		m_pLastCluster = pCluster;
	}

	pwszAddedString = pCluster->AddString( pwszString );

	if( pwszAddedString ) {
		m_nStringCount++;
	}

	return pwszAddedString;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFStringCluster
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CFStringCluster::CFStringCluster( void ) {
	m_nMaxStringCount = 0;
	m_nStringCount = 0;
	m_ppStringArray = NULL;
	m_paStringBitTypes = NULL;
}


CFStringCluster *CFStringCluster::CreateCluster( void ) {
	CFStringCluster *pCluster;

//ARG - >>>>>
//ARG - initialization crosses 'goto' scope
	u32 nBitArraySize;
//ARG - <<<<<
	
	FResFrame_t ResFrame = fres_GetFrame();
	pCluster = NULL;

	pCluster = fnew CFStringCluster;
	if( pCluster == NULL ) {
		DEVPRINTF( "CFStringCluster::CreateCluster(): Not enough memory to allocate string cluster.\n" );
		goto _ExitWithError;
	}

	pCluster->m_nMaxStringCount = Fang_ConfigDefs.nStringTable_DefaultMaxStringCountPerCluster;
	pCluster->m_ppStringArray = (void **) fres_AllocAndZero( pCluster->m_nMaxStringCount * sizeof( void* ) );
	if( pCluster->m_ppStringArray == NULL ) {
		DEVPRINTF( "CFStringCluster::CreateCluster(): Not enough memory to allocate string cluster.\n" );
		goto _ExitWithError;
	}

	nBitArraySize = ( Fang_ConfigDefs.nStringTable_DefaultMaxStringCountPerCluster / 8 ) + 
		!!( ( Fang_ConfigDefs.nStringTable_DefaultMaxStringCountPerCluster % 8 ) > 0 );
	pCluster->m_paStringBitTypes = ( u8* )fres_AllocAndZero( nBitArraySize );
	if( pCluster->m_paStringBitTypes == NULL ) {
		DEVPRINTF( "CFStringCluster::CreateCluster(): Not enough memory to allocate string cluster.\n" );
		goto _ExitWithError;
	}


	return pCluster;

	// Error:
_ExitWithError:
	fdelete( pCluster );
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


cchar *CFStringCluster::AddString( cchar *pszString ) {
	char *pszClusterString;

	FASSERT( m_nStringCount < m_nMaxStringCount );

	pszClusterString = (char *)fres_Alloc( fclib_strlen(pszString) + 1 );
	if( pszClusterString == NULL ) {
		DEVPRINTF( "CFStringCluster::AddString(): Not enough memory to add string.\n" );
		return NULL;
	}

	fclib_strcpy( pszClusterString, pszString );

	m_ppStringArray[m_nStringCount++] = pszClusterString;

	//NOTE : There is no need to set the bit value for this string 
	//because all bits for this cluster are cleared by default when
	//the cluster is being created.

	return pszClusterString;
}


cwchar *CFStringCluster::AddString( cwchar *pwszString ) {
	wchar *pwszClusterString;

	FASSERT( m_nStringCount < m_nMaxStringCount );

	pwszClusterString = (wchar *)fres_Alloc( ( fclib_wcslen( pwszString ) + 1 ) * sizeof( wchar ) );
	if( pwszClusterString == NULL ) {
		DEVPRINTF( "CFStringCluster::AddString(): Not enough memory to add string.\n" );
		return NULL;
	}

	fclib_wcscpy( pwszClusterString, pwszString );

	u32 nByte = m_nStringCount / 8;
	u32 nBit = m_nStringCount % 8;

	m_paStringBitTypes[ nByte ] |= ( 1 << nBit ); //set the bit for WIDESTRING
	m_ppStringArray[m_nStringCount++] = pwszClusterString;

	return pwszClusterString;
}


cchar *CFStringCluster::FindString( cchar *pszString ) const {
	u32 i;

	u32 nBitCounter = 0;
	u32 nByteCounter = 0;
	u32 uMask;
	if( Fang_ConfigDefs.bStringTable_CaseSensativeStringNames ) {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( !( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_strcmp( pszString, (cchar *)m_ppStringArray[i] ) ) {
				return (cchar *)m_ppStringArray[i];
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	} else {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( !( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_stricmp( pszString, (cchar *)m_ppStringArray[i] ) ) {
				return (cchar *)m_ppStringArray[i];
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	}

	return NULL;
}


cwchar *CFStringCluster::FindString( cwchar *pwszString ) const {
	u32 i;

	u32 nBitCounter = 0;
	u32 nByteCounter = 0;
	u32 uMask;
	if( Fang_ConfigDefs.bStringTable_CaseSensativeStringNames ) {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( ( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_wcscmp( pwszString, (wchar*)m_ppStringArray[i] ) ) {
				return (cwchar *)m_ppStringArray[i];
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	} else {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( ( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_wcsicmp( pwszString, (cwchar *)m_ppStringArray[i] ) ) {
				return (cwchar *)m_ppStringArray[i];
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	}

	return NULL;
}


s32 CFStringCluster::ComputeStringIndex( cchar *pszString ) const {
	u32 i;

	u32 nBitCounter = 0;
	u32 nByteCounter = 0;
	u32 uMask;
	if( Fang_ConfigDefs.bStringTable_CaseSensativeStringNames ) {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( !( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_strcmp( pszString, (cchar *)m_ppStringArray[i] ) ) {
				return (s32)i;
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	} else {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( !( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_stricmp( pszString, (cchar *)m_ppStringArray[i] ) ) {
				return (s32)i;
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	}

	return -1;
}


s32 CFStringCluster::ComputeStringIndex( cwchar *pwszString ) const {
	u32 i;

	u32 nBitCounter = 0;
	u32 nByteCounter = 0;
	u32 uMask;
	if( Fang_ConfigDefs.bStringTable_CaseSensativeStringNames ) {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( ( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_wcscmp( pwszString, (wchar*)m_ppStringArray[i] ) ) {
				return (s32)i;
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	} else {
		for( i=0; i<m_nStringCount; i++ ) {
			uMask = 1 << nBitCounter;
			if( ( uMask & m_paStringBitTypes[nByteCounter] ) && !fclib_wcsicmp( pwszString, (cwchar *)m_ppStringArray[i] ) ) {
				return (s32)i;
			}
			nBitCounter++;
			if( nBitCounter == 8 ) {
				nByteCounter++;
				nBitCounter = 0;
			}
		}
	}

	return -1;
}

