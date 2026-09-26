//////////////////////////////////////////////////////////////////////////////////////
// ItemRepository.cpp - Repository of item data for Mettle Arms.
//
// Author: Justin Link
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
// 02/11/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "ItemRepository.h"
#include "fworld.h"
#include "Item.h"
#include "ffile.h"
#include "fclib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


u32 CItemRepository::m_nItemCount;
CItem *CItemRepository::m_paItems;
BOOL CItemRepository::m_bIsInitialized = FALSE;


BOOL CItemRepository::InitSystem() {
	FASSERT(!m_bIsInitialized);
	m_bIsInitialized = TRUE;
	m_paItems = NULL;
	m_nItemCount = 0;

	return InitFromCSV("ItemList$");
}


void CItemRepository::UninitSystem() {

	if( m_bIsInitialized ) {
        m_bIsInitialized = FALSE;
		if( m_paItems ) {
			fdelete_array( m_paItems );
		}
		m_paItems = NULL;
        m_nItemCount = 0;
	}
}


BOOL CItemRepository::InitFromCSV( cchar *pszFileName ) {
	FGameData_VarType_e eDataType;
	FGameDataTableHandle_t hTableHandle;
	ItemInit_s oItemInit;
	u32 uLineNum = 0;
	u32 uFieldNum, nNumTables;
	FGameDataWalker_t hWalker;

	FASSERT( m_bIsInitialized );
	FASSERT( !m_paItems );

	FMemFrame_t hFMemFrame = fmem_GetFrame();
	FResFrame_t hFResFrame = fres_GetFrame();

	// Read the .CSV file here.
	FGameDataFileHandle_t hFile = (FGameDataFileHandle_t)fgamedata_LoadFileToFMem( pszFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF("CItemRepository::InitFromCSV() : Something went wrong opening CSV file '%s'.  Cannot continue.\n", pszFileName);
		goto _ExitWithError;
	}

	// count how many tables there are in the csv file
	hTableHandle = fgamedata_GetFirstTable(hFile, hWalker);
	nNumTables = 0;
	while( hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		nNumTables++;
		hTableHandle = fgamedata_GetNextTable( hWalker );
	}
	if( !nNumTables ) {
		DEVPRINTF("CItemRepository::InitFromCSV() : No tables in CSV file '%s'.  Cannot continue.\n", pszFileName );
		goto _ExitWithError;
	}
	// allocate enough items for all of the tables
	m_paItems = fnew CItem[nNumTables];
	if( !m_paItems ) {
		DEVPRINTF("CItemRepository::InitFromCSV() : Could not allocate %d items.  Cannot continue.\n", nNumTables );
		goto _ExitWithError;
	}
	// allocate some temp memory to record strings
	char **papszUpgradeNames;
	papszUpgradeNames = (char **)fmem_AllocAndZero( sizeof( char * ) * nNumTables, 4 );
	if( !papszUpgradeNames ) {
		DEVPRINTF("CItemRepository::InitFromCSV() : Could not allocate temp memory.  Cannot continue.\n" );
		goto _ExitWithError;
	}
	
	////////////////////////////////////////////////////////////////////
	// Walk through each table sequentially.
	hTableHandle = fgamedata_GetFirstTable(hFile, hWalker);
	m_nItemCount = 0;
	while( hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		++uLineNum;
		uFieldNum = 0;

		////////////////////////////////////////////////////////////////////
		// Tag field.
		oItemInit.pszTag = (cchar *)(fgamedata_GetTableName(hTableHandle));
//		if(eDataType != FGAMEDATA_VAR_TYPE_STRING)
//		{
//			DEVPRINTF("CItemRepository::InitFromCSV() : Error in tag field, line %d, wrong data type.\n", uLineNum);
//			goto _ExitWithError;
//		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Code name.
		oItemInit.pszCodeName = (cchar *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in name field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Display name.
		oItemInit.pwszDisplayName = (cwchar *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_WIDESTRING)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in name field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Long description.
		oItemInit.pwszLongDesc = (cwchar *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_WIDESTRING)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in long description field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Upgrade description.
		oItemInit.pwszUpgradeDesc = (cwchar *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_WIDESTRING)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in upgade description field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Upgrade item.
		cchar *pszTemp = (cchar *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in upgade item field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		if( fclib_stricmp(pszTemp, "None") != 0 ) {
			papszUpgradeNames[m_nItemCount] = (char *)pszTemp;
		} else {
			papszUpgradeNames[m_nItemCount] = NULL;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// TGA file.
		oItemInit.pszIconTGAFile = (cchar *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in icon TGA field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		if(fclib_stricmp(oItemInit.pszIconTGAFile, "None") == 0)
		{
			oItemInit.pszIconTGAFile = NULL;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Mesh file.
		oItemInit.pszMeshFile = (cchar *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in mesh field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		if(fclib_stricmp(oItemInit.pszMeshFile, "None") == 0)
		{
			oItemInit.pszMeshFile = NULL;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// TCs.
		oItemInit.vecST1.x = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in TC field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		oItemInit.vecST1.y = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in TC field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		oItemInit.vecST2.x = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in TC field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		oItemInit.vecST2.y = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in TC field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Inventory positions.
		oItemInit.uInventoryPos_x = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in inventory pos x field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		oItemInit.uInventoryPos_y = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in inventory pos y field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// EUK levels.
		oItemInit.uCurLevel = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in cur EUK level field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		oItemInit.uMaxLevel = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in max EUK level field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		oItemInit.uFlags = ITEMFLAG_NONE;

		////////////////////////////////////////////////////////////////////
		// Primary flag.
		u32 uTemp = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in primary item flag field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		if(uTemp == 1)
		{
			oItemInit.uFlags |= ITEMFLAG_PRIMARY;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Secondary flag.
		uTemp = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in secondary item flag field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		if(uTemp == 1)
		{
			oItemInit.uFlags |= ITEMFLAG_SECONDARY;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Item type.
		oItemInit.uItemType = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in item type field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////
/*
		////////////////////////////////////////////////////////////////////
		// Z-offset (to us).
		oItemInit.fZOffset = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in y-offset field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////
*/
		oItemInit.fZOffset = 0.0f;

		////////////////////////////////////////////////////////////////////
		// Y-offset (to us).
		oItemInit.fYOffset = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in z-offset field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Scale factor.
		oItemInit.fScale = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in scale field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Pitch angle.
		oItemInit.fPitch_DEG = *(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in pitch angle field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		//
		////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////
		// Postering flag.
		uTemp = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData(hTableHandle, uFieldNum++, eDataType)));
		if(eDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CItemRepository::InitFromCSV() : Error in mesh postering field, line %d, tag %s, wrong data type.\n", uLineNum, oItemInit.pszTag);
			goto _ExitWithError;
		}
		if(uTemp == 1)
		{
			oItemInit.bPoster = TRUE;
		}
		else
		{
			oItemInit.bPoster = FALSE;
		}
		//
		////////////////////////////////////////////////////////////////////

		m_paItems[m_nItemCount].Init( &oItemInit );

		++m_nItemCount;
				
		hTableHandle = fgamedata_GetNextTable(hWalker);
	}
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	// Here we go through and back-patch the upgraded weapon information into the weapons we just loaded.
	u32 uCurItemIdx;
	CItem *pUpgradeItem, *pCurItem;
	for( uCurItemIdx = 0; uCurItemIdx < m_nItemCount; ++uCurItemIdx ) {
		if( papszUpgradeNames[uCurItemIdx] ) {
			pUpgradeItem = RetrieveEntry( papszUpgradeNames[uCurItemIdx], NULL);
			if( pUpgradeItem != NULL ) {
				pCurItem = &m_paItems[uCurItemIdx];

				pCurItem->m_pUpgraded = pUpgradeItem;
				pUpgradeItem->m_pDowngraded = pCurItem;
			} else {
				DEVPRINTF("Could not find upgrade item '%s' for item '%s'.\n", papszUpgradeNames[uCurItemIdx], m_paItems[uCurItemIdx].m_pszCodeName);
			}	
		}
	}
	////////////////////////////////////////////////////////////////////

	fmem_ReleaseFrame( hFMemFrame );

	return(TRUE);

	// Error:
_ExitWithError:
	fmem_ReleaseFrame( hFMemFrame );
	if( m_paItems ) {
		fdelete_array( m_paItems );
		m_paItems = NULL;
	}
	fres_ReleaseFrame( hFResFrame );

	return(FALSE);
}


CItem *CItemRepository::RetrieveEntry( cchar *pszItemTag, u32 *puIndex ) {
	FASSERT(m_bIsInitialized);

	u32 uCurIdx;
	for( uCurIdx = 0; uCurIdx < m_nItemCount; ++uCurIdx ) {
		if( fclib_stricmp(m_paItems[uCurIdx].m_pszTag, pszItemTag) == 0 ) {
			if( puIndex != NULL ) {
				*puIndex = uCurIdx;
			}
			return &m_paItems[uCurIdx];
		}
	}

	return NULL;
}

// uEUKLevel is 0 based.
CItem *CItemRepository::RetrieveEntry( cchar *pszItemName, u32 uEUKLevel, u32 *puIndex ) {
	FASSERT(m_bIsInitialized);

	++uEUKLevel;

	u32 uCurIdx;
	for( uCurIdx = 0; uCurIdx < m_nItemCount; ++uCurIdx ) {
		if( fclib_stricmp(m_paItems[uCurIdx].m_pszCodeName, pszItemName) == 0 ) {
			if( ( m_paItems[uCurIdx].m_uCurLevel == uEUKLevel ) ||
				( m_paItems[uCurIdx].m_uMaxLevel == 0 ) ) {
				if( puIndex != NULL ) {
					*puIndex = uCurIdx;
				}
				return &m_paItems[uCurIdx];
			}
		}
	}

	return NULL;
}

CItem *CItemRepository::RetrieveEntry( u32 nIndex ) {
	FASSERT(m_bIsInitialized);
	
	if( nIndex < m_nItemCount ) {
		return &m_paItems[nIndex];
	}
	return NULL;
}

u32 CItemRepository::GetNumItems() {
	FASSERT(m_bIsInitialized);

	return m_nItemCount;
}
