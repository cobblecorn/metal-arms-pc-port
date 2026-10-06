//////////////////////////////////////////////////////////////////////////////////////
// BarterTypes.cpp - 
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
// 09/03/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BarterTypes.h"

#include "botglitch.h"
#include "meshentity.h"
#include "fresload.h"
#include "fclib.h"
#include "floop.h"
#include "gstring.h"
#include "item.h"
#include "iteminst.h"
#include "fsndfx.h"
#include "TalkSystem2.h"
#include "shady.h"
#include "BarterSystem.h"
#include "weapon.h"
#include "botglitch.h"
#include "collectable.h"
#include "miscdraw.h"
#include "meshtypes.h"
#include "game.h"

static const f32 SELECTED_ITEM_YAW_FREQ = 2.0f;


static cchar* BARTERITEMS_SLOT_TABLE[BARTER_TYPES_NUM_SLOTS] = {"Items","Ammo","Weapons"};
static const u32 _nSlotIterateOrder[] = { 2, 1, 0, -1 };
static const u32 AMMO_SLOT				= 1;

static const u32 NUMFIELDS_PER_ITEM		= 7;
static const u32 NUMFIELDS_PER_TABLE	= 4;
static const u32 NUMFIELDS_PER_CHAIN	= 6;
static const u32 NUMFIELDS_PER_ITEMINST = 3;

static void _InitSortList(u8* paSortArray, CBarterResponse* paResponses, u32 nNumResponses);
static void _ScrambleArray(u8* pauArray, u32 uSize, u32 uScramblePasses);


///////////////////////////
// CBarterResponse Methods
///////////////////////////

CBarterResponse::CBarterResponse() {
	m_pInst = NULL;
	m_nShadyState = 0;
	m_nPercentChance = 100;
	m_pszBarterFilename = NULL;
	m_nEventID = 0;
	m_nPercentCompleteToFireOffEvent = 0;
	m_bEventFired = 0;
}

CBarterResponse::~CBarterResponse() {
	if( m_pInst ) {
		fdelete( m_pInst );
		m_pInst = NULL; 
	}
	m_pszBarterFilename = NULL;
}

BOOL CBarterResponse::Init( cchar *pszFilename, f32 fPercent, cchar *pszKeyword, cchar *pszEventName, f32 fPercentCompleteBeforeEventTriggers ) {
	
	if( m_pInst ) {
		fdelete( m_pInst );
		m_pInst = NULL; 
	}
	u32 i;
	
	if( !pszFilename || !pszKeyword ) {
		return FALSE;
	}

	// Find the keyword. Retail data has a special no-money response tagged
	// NOSOUPFORYOU; it belongs to the existing purchase-abort state.
	if( fclib_stricmp( pszKeyword, "NOSOUPFORYOU" ) == 0 ) {
		m_nShadyState = CShady::SHADYSTATE_PURCHASEABORT;
	} else {
		for( i=0; i < CShady::SHADYSTATE_COUNT; i++ ) {
			if( fclib_stricmp( Shady_aBotTalkInfo[i].pszKeyword, pszKeyword ) == 0 ) {
				// found a match
				m_nShadyState = Shady_aBotTalkInfo[i].nStateValue;
				break;
			}
		}
		if( i == CShady::SHADYSTATE_COUNT ) {
			goto _ExitWithError;
		}
	}

	// set the probablity
	FMATH_CLAMP( fPercent, 0.0f, 100.0f );
	m_nPercentChance = (u8)fPercent;

	// add the keyword to perm memory
	m_pszBarterFilename = gstring_Main.AddString( pszFilename );

	// find the event keyword
	for( i=0; i < BARTER_SYSTEM_EVENTS_SLIM_COUNT; i++ ) {
		if( fclib_stricmp( BarterSystem_apszEventStrings[i], pszEventName ) == 0 ) {
			// found a match
			m_nEventID = (u8) i;
			break;
		}
	}
	if( i == BARTER_SYSTEM_EVENTS_SLIM_COUNT ) {
		goto _ExitWithError;
	}

	// set the event complete percent
	FMATH_CLAMP( fPercentCompleteBeforeEventTriggers, 0.0f, 100.0f );
	m_nPercentCompleteToFireOffEvent = (u8)fPercentCompleteBeforeEventTriggers;
	m_bEventFired = 0;

	return TRUE;

_ExitWithError:
	return FALSE;
}

BOOL CBarterResponse::Load() {

	FASSERT( m_pszBarterFilename );
	if( m_pInst ) {
		// already loaded
		return TRUE;
	}
	
	FResFrame_t hResFrame = fres_GetFrame();
	FMemFrame_t hMemFrame = fmem_GetFrame();

	// load the bot talk file
	CBotTalkData * pBTA;
	pBTA = CTalkSystem2::GetTalkDataByFileName( m_pszBarterFilename );
	if( !pBTA ) {
		goto _ExitWithError;
	}
	// create a bot talk inst
	m_pInst = fnew CTalkInst();
	if( !m_pInst ) {
		goto _ExitWithError;
	}
	if( !m_pInst->Init( pBTA ) ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	if( m_pInst ) {
		fdelete( m_pInst );
		m_pInst = NULL;
	}
	fres_ReleaseFrame( hResFrame );
	fmem_ReleaseFrame( hMemFrame );

	return FALSE;
}


static void _InitSortList(u8* paSortArray, CBarterResponse* paResponses, u32 nNumResponses)
{
	//Initialize Sort Order
	u8 i, j;
	for (i = 0; i < nNumResponses; i++)
	{
		paSortArray[i] = i;
	}

	//bubble sort (load-time and small arrays ( < 20)
	for (i = 0; i < nNumResponses; i++)
	{
		for (j = i+1; j < nNumResponses; j++)
		{
			if (paResponses[paSortArray[j]].m_nShadyState < paResponses[paSortArray[i]].m_nShadyState)
			{
			   u8 tmp = paSortArray[j];
			   paSortArray[j] = paSortArray[i];
			   paSortArray[i] = tmp;
			}
		}
	}

	for (i = 0; i < CShady::SHADYSTATE_COUNT; i++)
	{
		BarterTypes_ScrambleSortList(paSortArray, paResponses, nNumResponses, i, NULL);
	}
}


static void _ScrambleArray(u8* pauArray, u32 uSize, u32 uScramblePasses)
{
	u8 tmp;
	u32 uPass;
	u32 i, j;
	if (uSize > 1)
	{
		u32 uRandStep = 1+fmath_RandomChoice(uSize-1);
		for (uPass = 0; uPass < uScramblePasses; uPass++)
		{
			for (i = 0; i < uSize; i++)
			{
				j = (i+uRandStep);
				if (j >= uSize)
				{
					j-=uSize;
				}
				tmp = pauArray[i];
				pauArray[i] = pauArray[j];
				pauArray[j] = tmp;
			}
		}
	}
}


void BarterTypes_ScrambleSortList(u8* paSortArray, CBarterResponse* paResponses, u32 nNumResponses, u32 uShadyState, CBarterResponse *pPleaseNotFirst)
{
	u8 tmp;
	s32 nStart = -1;
	s32 nEnd = -1;
	//Initialize Sort Order
	u32 i;
	
	for (i = 0; i < nNumResponses; i++)
	{
		if (paResponses[paSortArray[i]].m_nShadyState == uShadyState)
		{
			if (nStart == -1)
			{
				nStart = i;
			}
		}

		if (nStart !=-1 && paResponses[paSortArray[i]].m_nShadyState != uShadyState)
		{
			nEnd = i-1;
			break;
		}
	}

	if (nStart != -1 &&
		nEnd > nStart)
	{
		_ScrambleArray(paSortArray+nStart, (nEnd - nStart)+1, 5);
	}

	if (pPleaseNotFirst && 
		pPleaseNotFirst == &(paResponses[paSortArray[nStart]]))
	{
	   		tmp = paSortArray[nStart];
			paSortArray[nStart] = paSortArray[nEnd];
			paSortArray[nEnd] = tmp;
	}
}


///////////////////////////
// CBarterItem Methods
///////////////////////////

u32 CBarterItem::m_aNumItems[BARTER_TYPES_NUM_SLOTS];
CBarterItem * CBarterItem::m_apaItems[BARTER_TYPES_NUM_SLOTS];

u32 CBarterItem::m_nNumGenericResponses;
CBarterResponse *CBarterItem::m_paGenericResponses = NULL;
u8 *CBarterItem::m_pauGenericPlayFlags = NULL;
u8 *CBarterItem::m_pauGenericSortOrder = NULL;

CBarterItem::CBarterItem() {
	m_fScale = 1.0f;
	m_nBasePrice = 1;
	m_vTableOffset.Zero();
	m_nNumResponses = 0;
	m_paResponses = NULL;
	m_pauPlayFlags = NULL;
	m_pauSortOrder = NULL;
}

CBarterItem::~CBarterItem() 
{
	if( m_paResponses ) 
	{
		fdelete_array( m_paResponses );
		m_paResponses = NULL;
	}	

	m_pauPlayFlags = NULL;	   //fres_alloc'd
	m_pauSortOrder = NULL;	   //fres_alloc'd
}

BOOL CBarterItem::InitFromFile( cchar *pszCSVFile ) {
	FGameData_VarType_e nDataType;
	FGameDataTableHandle_t hTableHandle, hBotTalkTable;
	cchar *pszItemName, *pszResponseTable;
	u32 uPrice, uNumFields, nNumItems, uCurFieldIdx, nNumResponses;
	f32 fScale;
	CBarterResponse *paResponse;
	CFVec3A ObjectOffset;
	
	FASSERT( pszCSVFile );

	m_nNumGenericResponses = 0;
	m_paGenericResponses = NULL;
	m_pauGenericPlayFlags = NULL;
	m_pauGenericSortOrder = NULL;

	FResFrame_t hResFrame = fres_GetFrame();
	FMemFrame_t hMemFrame = fmem_GetFrame();

	// Read the .CSV file here.
	FGameDataFileHandle_t hFile = fgamedata_LoadFileToFMem( pszCSVFile );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) 
	{
		DEVPRINTF( "CBarterItem::InitFromFile() : Unable to open CSV file '%s'.\n", pszCSVFile );
		goto _ExitWithError;
	}

	//	for (s32 nSlot = 0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++)
	u32 i = 0;
	for(s32 nSlot=_nSlotIterateOrder[i]; nSlot != -1 ; i++, nSlot = _nSlotIterateOrder[i] ) 
	{
		// find the table we are interested in
		hTableHandle = fgamedata_GetFirstTableHandle( hFile, BARTERITEMS_SLOT_TABLE[nSlot] );
		if( hTableHandle == FGAMEDATA_INVALID_TABLE_HANDLE ) 
		{
			DEVPRINTF( "CBarterItem::InitFromFile() : No '%s' table found in '%s'.\n", BARTERITEMS_SLOT_TABLE[nSlot], pszCSVFile );
			goto _ExitWithError;
		}

		// get the number of fields in the table
		uNumFields = fgamedata_GetNumFields( hTableHandle );
		nNumItems = uNumFields / NUMFIELDS_PER_ITEM;
		if( (nNumItems * NUMFIELDS_PER_ITEM) != uNumFields ) 
		{
			DEVPRINTF( "CBarterItem::InitFromFile() : Too many items for table '%s' in file '%s'.\n", BARTERITEMS_SLOT_TABLE[nSlot], pszCSVFile );
			goto _ExitWithError;
		}
			
		// allocate enough items
		m_apaItems[nSlot] = NULL;
		m_apaItems[nSlot] = fnew CBarterItem[nNumItems];
		if( !m_apaItems[nSlot] ) 
		{
			DEVPRINTF( "CBarterItem::InitFromFile() : Could not allocate enough memory to hold all of the listed barter items.\n" );
			goto _ExitWithError;
		}

		// walk the csv file initing items
		uCurFieldIdx = 0;
		m_aNumItems[nSlot] = 0;
		while( uCurFieldIdx < uNumFields ) 
		{
			pszItemName	   =(cchar *)fgamedata_GetPtrToFieldData( hTableHandle, uCurFieldIdx + 0, nDataType );
			uPrice = (u32)*(  (f32 *)fgamedata_GetPtrToFieldData( hTableHandle, uCurFieldIdx + 1, nDataType ) );
			fScale         = *(f32 *)fgamedata_GetPtrToFieldData( hTableHandle, uCurFieldIdx + 2, nDataType );
			ObjectOffset.x = *(f32 *)fgamedata_GetPtrToFieldData( hTableHandle, uCurFieldIdx + 3, nDataType );
			ObjectOffset.y = *(f32 *)fgamedata_GetPtrToFieldData( hTableHandle, uCurFieldIdx + 4, nDataType );
			ObjectOffset.z = *(f32 *)fgamedata_GetPtrToFieldData( hTableHandle, uCurFieldIdx + 5, nDataType );
			pszResponseTable=(cchar *)fgamedata_GetPtrToFieldData( hTableHandle, uCurFieldIdx + 6, nDataType );

			// see if no bot talk table is specified
			if( fclib_stricmp( pszResponseTable, "none" ) == 0 ) 
			{
				pszResponseTable = NULL;
				nNumResponses = 0;
				paResponse = NULL;			
			} 
			else 
			{
				// find the bot talk table
				hBotTalkTable = fgamedata_GetFirstTableHandle( hFile, pszResponseTable );
				if( hTableHandle == FGAMEDATA_INVALID_TABLE_HANDLE ) 
				{
					DEVPRINTF( "CBarterItem2::InitFromFile() : Table '%s' could not be found in file '%s'.\n", pszResponseTable, pszCSVFile );
					goto _ExitWithError;
				}
				paResponse = CreateResponseArrayFromTable( hBotTalkTable, nNumResponses );
				if( !paResponse ) 
				{
					DEVPRINTF( "CBarterItem2::InitFromFile() : Trouble turning the table '%s' into a valid bot talk response array, check file '%s'.\n", pszResponseTable, pszCSVFile );
					goto _ExitWithError;
				}
			}

			if( !m_apaItems[nSlot][m_aNumItems[nSlot]].Init( pszItemName, uPrice, fScale, &ObjectOffset,nNumResponses,paResponse ) ) 
			{
				goto _ExitWithError;
			}

			m_aNumItems[nSlot]++;
			uCurFieldIdx += NUMFIELDS_PER_ITEM;
		}

	}
	// finally, init the array of generic bot talk responses
	hBotTalkTable = fgamedata_GetFirstTableHandle( hFile, "Generic_Bot_Talks" );
	if( hTableHandle == FGAMEDATA_INVALID_TABLE_HANDLE ) 
	{
		DEVPRINTF( "CBarterItem::InitFromFile() : File '%s' has no 'Generic_Bot_Talks' table, there will be no generic bot talks.\n", pszCSVFile );				
	} else 
	{
		paResponse = CreateResponseArrayFromTable( hBotTalkTable, nNumResponses );
		if( !paResponse ) 
		{
			DEVPRINTF( "CBarterItem::InitFromFile() : Trouble turning the table 'Generic_Bot_Talks' into a valid bot talk response array.\n" );
			goto _ExitWithError;
		}
		m_nNumGenericResponses = nNumResponses;
		m_paGenericResponses = paResponse;

		m_pauGenericPlayFlags = (u8*) fres_AlignedAllocAndZero(sizeof(u8)*nNumResponses, 16);
		m_pauGenericSortOrder = (u8*) fres_AlignedAlloc(sizeof(u8)*nNumResponses, 16);

		_InitSortList(m_pauGenericSortOrder, m_paGenericResponses, nNumResponses);

		if (!m_pauGenericPlayFlags || !m_pauGenericSortOrder)
		{
			DEVPRINTF( "CBarterItem::InitFromFile() : No Mem Yo!, talk to John");
			goto _ExitWithError;
		}
	}	

	fmem_ReleaseFrame( hMemFrame );

	return TRUE;

_ExitWithError:
	for (s32 nSlot = 0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++)
	{
		if( m_apaItems[nSlot] ) 
		{
			fdelete_array( m_apaItems[nSlot] );
			m_apaItems[nSlot] = NULL;
			m_aNumItems[nSlot] = 0;
		}
	}
	fres_ReleaseFrame( hResFrame );
	fmem_ReleaseFrame( hMemFrame );

	return FALSE;
}

void CBarterItem::Shutdown() 
{

	for (s32 nSlot = 0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++)
	{
		if( m_apaItems[nSlot] ) 
		{
			fdelete_array( m_apaItems[nSlot] );
			m_apaItems[nSlot] = NULL;
			m_aNumItems[nSlot] = 0;
		}
	}

	if( m_paGenericResponses ) 
	{
		fdelete_array( m_paGenericResponses );
		m_paGenericResponses = NULL;
		m_nNumGenericResponses = 0;
	}

	m_pauGenericPlayFlags = NULL;
	m_pauGenericSortOrder = NULL;
}

CBarterResponse *CBarterItem::CreateResponseArrayFromTable( FGameDataTableHandle_t hBotTalkTable, u32 &rnNumItems ) {

	rnNumItems = 0;
	u32 nNumFields = 0;
	u32 i, nIndex;
	CBarterResponse *paResponses = NULL;
	if( hBotTalkTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		nNumFields = fgamedata_GetNumFields( hBotTalkTable );
		rnNumItems = nNumFields / 5;
		if( (rnNumItems * 5) != nNumFields ) {
			DEVPRINTF( "CBarterItem::CreateResponseArrayFromTable() : Too many items for table '%s' in file '%s'.\n", 
				fgamedata_GetTableName( hBotTalkTable ),
				fgamedata_GetTableName( hBotTalkTable ) );
			return NULL;
		}
		// allocate an array of elements
		FResFrame_t hResFrame = fres_GetFrame();
		
		paResponses = fnew CBarterResponse[rnNumItems];
		if( !paResponses ) {
			DEVPRINTF( "CBarterItem::CreateResponseArrayFromTable() : Could not allocate memory for bot talk reponse data.\n" );
			return NULL;
		}
		
		FGameData_VarType_e nDataType;
		cchar *pszFilename, *pszKeyword, *pszEventName;
		f32 fPercent, fPercentCompleteBeforeEventTriggers;
		nIndex = 0;
		for( i=0; i < rnNumItems; i++ ) {
			pszFilename = (cchar *)fgamedata_GetPtrToFieldData( hBotTalkTable, nIndex, nDataType );
			pszKeyword = (cchar *)fgamedata_GetPtrToFieldData( hBotTalkTable, nIndex + 1, nDataType );
			fPercent = *(f32 *)fgamedata_GetPtrToFieldData( hBotTalkTable, nIndex + 2, nDataType );
			pszEventName = (cchar *)fgamedata_GetPtrToFieldData( hBotTalkTable, nIndex + 3, nDataType );
			fPercentCompleteBeforeEventTriggers = *(f32 *)fgamedata_GetPtrToFieldData( hBotTalkTable, nIndex + 4, nDataType );

			if( !paResponses[i].Init( pszFilename, fPercent, pszKeyword, pszEventName, fPercentCompleteBeforeEventTriggers ) ) {
				DEVPRINTF( "CBarterItem::CreateResponseArrayFromTable() : Could not init a bot talk reponse, File = '%s', Keyword = '%s'.\n", pszFilename, pszKeyword );
				fdelete_array( paResponses );
				fres_ReleaseFrame( hResFrame );
				return NULL;
			}
			nIndex += 5;
		}
		return paResponses;
	}
	return NULL;
}

CBarterItem *CBarterItem::FindBarterItem(u32 nSlot, cchar *pszItemName ) 
{
	for(u32 i=0; i < m_aNumItems[nSlot]; i++ ) 
	{
		if (m_apaItems[nSlot][i].m_eCollectableType == CCollectable::GetCollectableTypeFromString(pszItemName))
		{
			return &m_apaItems[nSlot][i];
		}
	}
	return NULL;
}

// set all CBarterResponse's to not loaded
void CBarterItem::UninitLevel() 
{
	CBarterResponse *pResponse;

	for (s32 nSlot = 0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++)
	{
		for (u32 nItem = 0; nItem < CBarterItem::m_aNumItems[nSlot]; nItem++)
		{
			for( u16 nResponses=0; nResponses < CBarterItem::m_apaItems[nSlot][nItem].m_nNumResponses; nResponses++ ) 
			{
				pResponse = &CBarterItem::m_apaItems[nSlot][nItem].m_paResponses[nResponses];
				if( pResponse->m_pInst ) 
				{
					fdelete( pResponse->m_pInst );
					pResponse->m_pInst = NULL; 
				}
			}
		}
	}
	for(u32 i=0; i < m_nNumGenericResponses; i++ ) 
	{
		pResponse = &m_paGenericResponses[i];
		if( pResponse->m_pInst ) 
		{
			fdelete( pResponse->m_pInst );
			pResponse->m_pInst = NULL;
		}
	}
}

BOOL CBarterItem::Init( cchar *pszItemName,
						const u32 uWasherPrice,
						f32 fScale/*=1.0f*/,
						CFVec3A *pOffset/*=NULL*/,
						u32 nNumResponses/*=0*/,
						CBarterResponse *paResponses/*=NULL*/ ) 
{ 
	// fill in our vars
	m_eCollectableType = CCollectable::GetCollectableTypeFromString( pszItemName );
	if (m_eCollectableType == COLLECTABLE_UNKNOWN)
	{
		DEVPRINTF( "CBarterItem::Init() : Failed to find item '%s' in collectables.  Item unknown.\n", pszItemName );
	}
	
	m_fScale = fScale;
	m_nBasePrice = uWasherPrice;
	m_nNumResponses = nNumResponses;
	m_paResponses = paResponses;

	m_pauPlayFlags = (u8*) fres_AlignedAllocAndZero(sizeof(u8)*m_nNumResponses, 16);
	m_pauSortOrder = (u8*) fres_AlignedAllocAndZero(sizeof(u8)*m_nNumResponses, 16);
	if (!m_pauPlayFlags || !m_pauSortOrder)
	{
		DEVPRINTF( "CBarterItem2::Init() : No Mem Yo!, talk to John");
		return FALSE;
	}
	_InitSortList(m_pauSortOrder, m_paResponses, m_nNumResponses);

	if( pOffset ) 
		m_vTableOffset = pOffset->v3;
	else 
		m_vTableOffset.Zero();

	return TRUE;
}

BOOL CBarterItem::LoadSharedResources(void)
{
	// NKM - Notify the collectable system that we may be giving this item out
	CCollectable::NotifyCollectableUsedInWorld( m_eCollectableType );

	return TRUE;
}

void CBarterItem::Purchase( CBot *pPlayerBot, u32 nFinalPrice ) const 
{
	FASSERT( pPlayerBot );
	FASSERT( pPlayerBot->TypeBits() & ENTITY_BIT_BOTGLITCH );
	
	// NKM
	//CWeapon *pWeapon;
	CBotGlitch *pGlitch = (CBotGlitch *)pPlayerBot;

	// assert that the player has enough money for this purchase
	FASSERT( pPlayerBot->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo >= (s16)nFinalPrice );

	// NKM
	CCollectable::GiveToPlayer( pGlitch, m_eCollectableType, 1.0f );
		
	// play a sound when you buy something
	FSndFx_FxHandle_t hItemDesc = fsndfx_GetFxHandle( "Power up Pick U" );
	fsndfx_Play2D( hItemDesc );

	// charge the player for the purchased item
	pPlayerBot->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo -= (s16)nFinalPrice;
}




//////////////////////////
// CBarterItemInst Methods
//////////////////////////

CBarterItemInst::CBarterItemInst() {
	m_poItem = NULL;
	m_pCollectableType = NULL;
	m_mtxToWorld.Identity();
	m_nBarterItemQuantity = 0;
	m_uBarterItemFlags = 0;
	m_nWasherPrice = 1;
	m_fAngle = 0.0f;
	m_fTimer = 0.0f;
	m_pWorldMesh = NULL;
}

CBarterItemInst::~CBarterItemInst() 
{
	if (IsInWorld())
		RemoveFromWorld();

	m_poItem = NULL;
	if( m_pCollectableType) 
	{
		m_pCollectableType = NULL;
	}
}

BOOL CBarterItemInst::Init( u32 nSlot, cchar *pszItemName, BOOL bLevelSpecified, s32 nHowMany, s32 nPrice)
{
	FASSERT( pszItemName != NULL );

	CBarterItem* pBarterItem = NULL;
	pBarterItem = CBarterItem::FindBarterItem( nSlot, pszItemName );
	if( pBarterItem == NULL ) 
	{
		DEVPRINTF( "CBarterItemInst::Init() : Failed to find item '%s' in slot '%d'.  Item unknown.\n", pszItemName, nSlot );
		return FALSE;
	}
	
	return Init(pBarterItem, bLevelSpecified, nHowMany, nPrice);
}

BOOL CBarterItemInst::Init( CBarterItem* pBarterItem, BOOL bLevelSpecified, s32 nHowMany, s32 nPrice)
{
	FASSERT(pBarterItem);

	m_poItem = pBarterItem;

	// load the bottalk files
	if( m_poItem->m_nNumResponses ) 
	{
		for(u32 nResponse=0; nResponse < m_poItem->m_nNumResponses; nResponse++ ) 
		{
			if( !m_poItem->m_paResponses[nResponse].Load() ) 
			{
				DEVPRINTF( "CBarterItemInst::Init() : Failed to load the needed bot talk file '%s'.\n", m_poItem->m_paResponses[nResponse].m_pszBarterFilename );
				return FALSE;
			}
		}
	}

	m_pCollectableType = CCollectable::GetCollectableTypeObjectFromType(m_poItem->m_eCollectableType);
	if( m_pCollectableType == NULL ) 
	{
		DEVPRINTF( "CBarterItemInst::Init() : Failed to GetCollectableTypeObject .\n" );
		return FALSE;
	}

	SetSelected(FALSE);

	SetLevelSpecified(bLevelSpecified);
	SetQuantity(nHowMany);
	    
	s32 nNewPrice = ((nPrice!=BARTER_DEFAULT_PRICE) ? nPrice : m_poItem->m_nBasePrice);
	if( nNewPrice <= 0 ) 
	{
		DEVPRINTF( "CBarterItemInst::Init() : Price of '%s' is less than 0, setting price to 1.\n", CCollectable::GetCollectableTypeObjectFromType(m_poItem->m_eCollectableType)->m_pszName);
		nNewPrice = 1;
	}
	m_nWasherPrice = (u16)nNewPrice;

	m_fAngle = 0.0f;
	m_fTimer = 0.0f;
	return TRUE;
}

static const CFVec3 _vEUKOffset(0.0f,0.0f,0.35f);
static const f32	_fEUKScale = .50f;

void CBarterItemInst::SetMtx( CFMtx43A &mtxNew ) 
{
	CFMtx43A mtxToWorld(mtxNew);
	
	CFVec3A vTableOffset;
	CFVec3A YOffset;
	f32 fScale;
	if (IsABox())
	{
		vTableOffset.Set(_vEUKOffset);
		fScale = _fEUKScale;
	}
	else
	{
		vTableOffset.Set(m_poItem->m_vTableOffset);
		fScale = m_poItem->m_fScale;
	}

	mtxToWorld.MulDir( YOffset, vTableOffset );
	mtxToWorld.m_vPos.Add( YOffset );	

	if (m_pWorldMesh)
	{
		m_pWorldMesh->m_Xfm.BuildFromMtx(mtxToWorld,fScale);
		m_pWorldMesh->UpdateTracker();
	}
	m_mtxToWorld.m_vRight = mtxToWorld.m_vRight.v3;
	m_mtxToWorld.m_vUp    = mtxToWorld.m_vUp.v3;
	m_mtxToWorld.m_vFront = mtxToWorld.m_vFront.v3;
	m_mtxToWorld.m_vPos   = mtxToWorld.m_vPos.v3;
}

void CBarterItemInst::AddToWorld() 
{
	if(IsInWorld() == FALSE) 
	{
		BOOL bIsEUKMesh = FALSE;
		if (IsAmmo())
		{
			m_pWorldMesh = CBarterLevel::m_pAmmoBoxMesh;
			CCollectableType::RemapTCs(m_pWorldMesh, m_pCollectableType->m_eType);
			SetABox(TRUE);
		}
		else
		{
			m_pWorldMesh = m_pCollectableType->GetWorldMesh(&bIsEUKMesh);
			SetABox(bIsEUKMesh);
		}
#if FANG_WINGC
		// Retry next frame if pickups or HUD animations temporarily occupy the display pool.
		if( !m_pWorldMesh ) {
			if( Fang_bPortDiag && FVid_nFrameCounter % 120 == 0 )
				DEVPRINTF( "Co-op shop: waiting for display mesh '%s'.\n", m_pCollectableType->m_pszName );
			return;
		}
#endif
		FASSERT(m_pWorldMesh);
		m_pWorldMesh->m_nFlags |= (FMESHINST_FLAG_NOSHADOWLOD|FMESHINST_FLAG_CAST_SHADOWS);
		m_pWorldMesh->AddToWorld();
		

		// Remove any lights that might be attached to this guy.
		CFWorldAttachedLight *pNextLight;
		CFWorldAttachedLight *pLight = m_pWorldMesh->GetFirstAttachedLight();
		while( pLight ) 
		{
			pNextLight = m_pWorldMesh->GetNextAttachedLight( pLight );

			m_pWorldMesh->RemoveAttachedLight( pLight );
			pLight->RemoveFromWorld();

			pLight = pNextLight;
		}
		
		// reset the timer to zero
		m_fAngle = 0.0f;
		m_fTimer = 0.0f;
		SetInWorld(TRUE);
	}
}

void CBarterItemInst::RemoveFromWorld(void) 
{
	if( IsInWorld() ) 
	{
		FASSERT(m_pWorldMesh);
		m_pWorldMesh->RemoveFromWorld();
        m_pWorldMesh->m_nFlags &= ~(FMESHINST_FLAG_NOSHADOWLOD|FMESHINST_FLAG_CAST_SHADOWS);
		if (!IsAmmo())
			m_pCollectableType->ReturnWorldMesh(m_pWorldMesh);

		m_pWorldMesh = NULL;

	//	SetSelected( FALSE );// can't be selected if we aren't in the world

		// reset the timer to zero
		m_fAngle = 0.0f;
		m_fTimer = 0.0f;
		SetInWorld(FALSE);
	}
}

void CBarterItemInst::Work() 
{
	CFMtx43A mtxFinal;
	mtxFinal.m_vRight.v3 = m_mtxToWorld.m_vRight;
	mtxFinal.m_vUp.v3	 = m_mtxToWorld.m_vUp;
	mtxFinal.m_vFront.v3 = m_mtxToWorld.m_vFront;
	mtxFinal.m_vPos.v3   = m_mtxToWorld.m_vPos;

	CFVec3A vNegativeScaledBoundSpherePos;

	if (IsInWorld()==FALSE)
		return;
	
	f32 fScale = m_poItem->m_fScale;
	if (IsABox())
	{
		fScale = _fEUKScale;
	}

	if( IsSelected() ) 
	{
		m_fTimer += FLoop_fPreviousLoopSecs;
		
		f32 fAngle = fmath_Sin( m_fTimer * SELECTED_ITEM_YAW_FREQ );
		m_fAngle = FMATH_FPOT( 0.35f, m_fAngle, fAngle );

		CFMtx43A mtxRotate = CFMtx43A::m_IdentityMtx;
		vNegativeScaledBoundSpherePos.Mul(m_pWorldMesh->m_BoundSphere_MS.m_Pos,-fScale);
		mtxRotate.m_vPos.Set(vNegativeScaledBoundSpherePos);
		mtxRotate.RotateY(m_fAngle);
		mtxRotate.m_vPos.Sub(vNegativeScaledBoundSpherePos);

		mtxFinal.Mul(mtxRotate);

 		FASSERT(m_pWorldMesh);
		m_pWorldMesh->m_Xfm.BuildFromMtx(mtxFinal,fScale);
		m_pWorldMesh->UpdateTracker();
		
		wchar pwszIntermediateOutputText[NUM_WCHARS_TAKE_OFF_THE_STACK];
		wchar pwszFinalOutputText[NUM_WCHARS_TAKE_OFF_THE_STACK];
		BOOL bok = TRUE;
		s32 nNumWChars;
		if (IsABox()) // need concat
		{
			if (IsAmmo())
			{
				switch(m_pCollectableType->m_eType)
				{
				default:
					_snwprintf( pwszIntermediateOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, Game_apwszPhrases[ GAMEPHRASE_N_ROUNDS_OF ], m_pCollectableType->m_nAmmoCount, L"");
					nNumWChars = fclib_wcslen(pwszIntermediateOutputText);
					bok = CCollectable::TypeToHUDString( &pwszIntermediateOutputText[nNumWChars], NUM_WCHARS_TAKE_OFF_THE_STACK-nNumWChars, m_pCollectableType, TRUE, TRUE );
					_snwprintf(pwszFinalOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, pwszIntermediateOutputText, Game_apwszPhrases[ GAMEPHRASE_AMMO ]);
					break;
				case COLLECTABLE_WEAPON_ROCKET_L1:
				case COLLECTABLE_WEAPON_ROCKET_L2:
				case COLLECTABLE_WEAPON_ROCKET_L3:
					_snwprintf( pwszFinalOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, Game_apwszPhrases[ GAMEPHRASE_N_ROCKETS ], m_pCollectableType->m_nAmmoCount);
					break;
				}
			}
			else // must be an upgrade
			{
				bok = CCollectable::TypeToHUDString( pwszIntermediateOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, m_pCollectableType, TRUE );
				_snwprintf(pwszFinalOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, pwszIntermediateOutputText, Game_apwszPhrases[ GAMEPHRASE_UPGRADE ]);
			}
		}
		else
		{
			switch(m_pCollectableType->m_eType)
			{
			default:
				bok = CCollectable::TypeToHUDString( pwszFinalOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, m_pCollectableType, FALSE );
				break;
			case COLLECTABLE_WEAPON_CORING_CHARGE:
				_snwprintf( pwszFinalOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, Game_apwszPhrases[ GAMEPHRASE_N_CORING_CHARGES ], m_pCollectableType->m_nAmmoCount);
				break;
			case COLLECTABLE_WEAPON_CLEANER:
				_snwprintf( pwszFinalOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, Game_apwszPhrases[ GAMEPHRASE_N_CLEANERS ], m_pCollectableType->m_nAmmoCount);
				break;
			case COLLECTABLE_WEAPON_EMP:
			case COLLECTABLE_WEAPON_MAGMA_BOMB:
			case COLLECTABLE_WEAPON_RECRUITER:
				_snwprintf( pwszFinalOutputText, NUM_WCHARS_TAKE_OFF_THE_STACK, L"%d ", m_pCollectableType->m_nAmmoCount);
				nNumWChars = fclib_wcslen(pwszFinalOutputText);
				bok = CCollectable::TypeToHUDString( &pwszFinalOutputText[nNumWChars], NUM_WCHARS_TAKE_OFF_THE_STACK-nNumWChars, m_pCollectableType, FALSE, FALSE);
				break;
			}
		}
		if (bok)
		{
           if (CBarterLevel::m_oTextArea.bVisible==FALSE)
			{
				CBarterLevel::m_oTextArea.bVisible = TRUE;
				ftext_SetAttributes(CBarterLevel::m_hTextDisplay,&CBarterLevel::m_oTextArea);
			}
			ftext_Printf( CBarterLevel::m_hTextDisplay,  L"~f1~C92929299~w0~aC~s1.00%ls", pwszFinalOutputText);
		}
	} 
	else if( m_fTimer > 0.0f ) 
	{
		// quickly animate to zero rot
		m_fAngle *= 0.85f;
		if( m_fAngle >= -0.01f && m_fAngle <= 0.01f ) 
		{
			m_fAngle = 0.0f;
			m_fTimer = 0.0f;
		}

		CFMtx43A mtxRotate = CFMtx43A::m_IdentityMtx;
		vNegativeScaledBoundSpherePos.Mul(m_pWorldMesh->m_BoundSphere_MS.m_Pos,-fScale);
		mtxRotate.m_vPos.Set(vNegativeScaledBoundSpherePos);
		mtxRotate.RotateY(m_fAngle);
		mtxRotate.m_vPos.Sub(vNegativeScaledBoundSpherePos);

		mtxFinal.Mul(mtxRotate);

		FASSERT(m_pWorldMesh);
		m_pWorldMesh->m_Xfm.BuildFromMtx(mtxFinal,fScale);
		m_pWorldMesh->UpdateTracker();
	}
}

BOOL CBarterItemInst::CanPurchase( CBot *pPlayerBot ) const {
	return ( pPlayerBot->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo >= (s32)m_nWasherPrice );
}

void CBarterItemInst::Purchase( CBot *pPlayerBot ) 
{
	m_poItem->Purchase( pPlayerBot, m_nWasherPrice );
	s8 nHowManyWeHave = GetQuantity();
	if (m_poItem->m_eCollectableType != COLLECTABLE_BATTERY) // battery's are fixed quantity!
	{
		if (nHowManyWeHave != BARTER_INFINITE)
		{
			FASSERT(nHowManyWeHave > 0);
			SetQuantity(nHowManyWeHave-1);
		}
	}
}




///////////////////////
// CBarterLevel Methods
///////////////////////

CBarterTable*		CBarterLevel::m_pCurrentTable = NULL;
u32					CBarterLevel::m_nNumTables = NULL;
CBarterTable*		CBarterLevel::m_paTables = NULL;
FTextArea_t			CBarterLevel::m_oTextArea;
FTextAreaHandle_t	CBarterLevel::m_hTextDisplay;
CFWorldMesh*		CBarterLevel::m_pAmmoBoxMesh;

BOOL CBarterLevel::InitLevel( cchar *pszCSVFile ) {
	FGameDataTableHandle_t hTables, hSlot;
	u32 nNumFields, nTable, j, nIndex, nItems, nNumEmptySlots;
	cchar *pszString[4], *pszChainName, *pszItemName, *pszUpsellItemName;
	FGameData_VarType_e nDataType;
	CBarterSlot *pSlot;
	CBarterSaleChains *pSaleChain;
	s32 nPrice,nCount,nUpsellItemPrice;

	FASSERT( !m_paTables );

	if( !pszCSVFile ) {
		return FALSE;
	}

	FMeshInit_t  MeshInit;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Identity();

	FResFrame_t hResFrame = fres_GetFrame();
	FMemFrame_t hMemFrame = fmem_GetFrame();

	// load the csv file
	FGameDataFileHandle_t hFile = fgamedata_LoadFileToFMem( pszCSVFile );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBarterLevel::InitLevel() : Unable to open CSV file '%s'.\n", pszCSVFile );
		goto _ExitWithError;
	}

	// find the table we are interested in
	hTables = fgamedata_GetFirstTableHandle( hFile, "barter_tables" );
	if( hTables == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBarterLevel::InitLevel() : No '%s' table found in '%s'.\n", "barter_tables", pszCSVFile );
		goto _ExitWithError;
	}

	// get the number of fields contained in the table
	nNumFields = fgamedata_GetNumFields( hTables );
	m_nNumTables = nNumFields / NUMFIELDS_PER_TABLE;
	if( (m_nNumTables * NUMFIELDS_PER_TABLE) != nNumFields ) 
	{
		DEVPRINTF( "CBarterLevel::InitLevel() : There are not the proper number of fields in the '%s' table found in '%s', there are '%d' fields per table.\n", "barter_tables", pszCSVFile, NUMFIELDS_PER_TABLE );
		goto _ExitWithError;
	}

	// allocate the table array
	m_paTables = fnew CBarterTable[m_nNumTables];
	if( !m_paTables ) {
		DEVPRINTF( "CBarterLevel::InitLevel() : Could not allocate memory for barter droid level data.\n" );
		goto _ExitWithError;
	}

	// walk the csv data, finding each slot table
	for( nTable=0; nTable < m_nNumTables; nTable++ ) 
	{
		// grab the string data for the i'th table
		nIndex = nTable * NUMFIELDS_PER_TABLE;
		nNumEmptySlots = 0;
		for( j=0; j < 4; j++ ) 
		{
			pszString[j] = (cchar *)fgamedata_GetPtrToFieldData( hTables, nIndex + j, nDataType );
			if( !pszString[j] || nDataType != FGAMEDATA_VAR_TYPE_STRING ) 
			{
				DEVPRINTF( "CBarterLevel::InitLevel() : Problem reading the table '%s'.\n", "barter_tables" );
				goto _ExitWithError;
			}
			// allow some of the last 3 strings to be == "empty"
			if( j > 0 ) 
			{
				if( fclib_stricmp( pszString[j], "empty" ) == 0 ) {
					pszString[j] = NULL;
					nNumEmptySlots++;
				}
			}
		}
		// make sure that all of the slots on the table aren't all empty
		if( nNumEmptySlots == BARTER_TYPES_NUM_SLOTS ) {
			DEVPRINTF( "CBarterLevel::InitLevel() : The table '%s' is defined with all %d slots empty, a table must have at least 1 slot offering items.\n", pszString[0], BARTER_TYPES_NUM_SLOTS );
			goto _ExitWithError;
		}

		// format of data representing each table:
		//	1st string is max name giving to the barter point
		//	2nd string is the table name for that slot 0
		//	3nd string is the table name for that slot 1
		//	4th string is the table name for that slot 2
		
		// add the max name to the gstring list
		m_paTables[nTable].m_pszName = gstring_Main.AddString( pszString[0] );
		u32 i = 0;
		for(s32 nSlot=_nSlotIterateOrder[i]; nSlot != -1 ; i++, nSlot = _nSlotIterateOrder[i] ) 
		{
			pSlot = &m_paTables[nTable].m_aSlots[nSlot];

			if( pszString[i+1] ) 
			{
				// get a handle to the slot table
				hSlot = fgamedata_GetFirstTableHandle( hFile, pszString[i+1] );
				if( hSlot == FGAMEDATA_INVALID_TABLE_HANDLE ) 
				{
					DEVPRINTF( "CBarterLevel::InitLevel() : No '%s' table found in '%s'.\n", pszString[nSlot+1], pszCSVFile );
					goto _ExitWithError;
				}

				// get the number of fields contained in the table
				nNumFields = fgamedata_GetNumFields( hSlot );
				u32 nNumChains = nNumFields / NUMFIELDS_PER_CHAIN;
				if( (nNumChains * NUMFIELDS_PER_CHAIN) != nNumFields ) 
				{
					DEVPRINTF( "CBarterLevel::InitLevel() : There are not the proper number of fields in the '%s' table found in '%s', there are '%d' fields per table.\n", pszString[nSlot+1], pszCSVFile, NUMFIELDS_PER_CHAIN );
					goto _ExitWithError;
				}

				// allocate  sales chains
#if FANG_WINGC
				// Room for every chain the level specifies plus every candidate item: sized to the candidates
				// alone, a level listing more chains (or one item twice) wrote past the array, and when the
				// fill below came up short the trailing chains kept no item, which the shop then read
				// (a crash opening Shady's shop in WESSstatn01). m_nSaleChains becomes the count filled.
				const u32 nChainCapacity = nNumChains + CBarterItem::m_aNumItems[nSlot];
#else
				const u32 nChainCapacity = CBarterItem::m_aNumItems[nSlot];
#endif
				pSlot->m_paSaleChains = fnew CBarterSaleChains[nChainCapacity];
				if( !pSlot->m_paSaleChains )
				{
					DEVPRINTF( "CBarterLevel::InitLevel() : Could not allocate memory for barter droid level data.\n" );
					goto _ExitWithError;
				}
				pSlot->SetChainIndex(0);
				pSlot->m_nSaleChains = nChainCapacity;
				pSlot->m_nSpecSaleChains =  nNumChains;

				u32 nSalesChain;
				for(nSalesChain=0; nSalesChain < pSlot->m_nSpecSaleChains; nSalesChain++ ) 
				{
					pSaleChain = &pSlot->m_paSaleChains[nSalesChain];

					pszChainName = (cchar *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+0, nDataType );
					if( !pszChainName || nDataType != FGAMEDATA_VAR_TYPE_STRING ) 
					{
						DEVPRINTF( "CBarterLevel::InitLevel() : Problem reading the table '%s'.\n", pszString[nSlot+1] );
						goto _ExitWithError;
					}
					nPrice = (s32)*( (f32 *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+1, nDataType ) );
					nCount = (s32)*( (f32 *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+2, nDataType ) );

					pszUpsellItemName = (cchar *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+3, nDataType );
					if( !pszUpsellItemName || (nDataType != FGAMEDATA_VAR_TYPE_STRING) || fclib_stricmp(pszUpsellItemName,"none")==0 ) 
					{
						pszUpsellItemName = NULL;
					}
					nUpsellItemPrice = (s32)*( (f32 *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+4, nDataType ) );
					nCount			 = (s32)*( (f32 *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+5, nDataType ) );

					nItems = (pszUpsellItemName ? 2 : 1);
					// allocate nItems CBarterItemInst
					pSaleChain->m_paItemInsts = fnew CBarterItemInst[nItems];
					if( !pSaleChain->m_paItemInsts ) 
					{
						DEVPRINTF( "CBarterLevel::InitLevel() : Could not allocate memory for barter droid level data.\n" );
						goto _ExitWithError;
					}
					pSaleChain->m_nNumItems = nItems;

					// walk each item inst and init it
					for( u32 nItemInst =0; nItemInst < nItems; nItemInst++ ) 
					{
						// read in the NUMFIELDS_PER_CHAIN fields from the csv file
						pszItemName       = (cchar *)fgamedata_GetPtrToFieldData( hSlot,       (nSalesChain*NUMFIELDS_PER_CHAIN)+(nItemInst*NUMFIELDS_PER_ITEMINST)+0, nDataType );
						nPrice            = (s32)*( (f32 *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+(nItemInst*NUMFIELDS_PER_ITEMINST)+1, nDataType ));
						nCount			  = (s32)*( (f32 *)fgamedata_GetPtrToFieldData( hSlot, (nSalesChain*NUMFIELDS_PER_CHAIN)+(nItemInst*NUMFIELDS_PER_ITEMINST)+2, nDataType ));
						if (nCount <=0)
							nCount = BARTER_INFINITE;

						if( !pSaleChain->m_paItemInsts[nItemInst].Init( nSlot, pszItemName, TRUE, nCount, nPrice ) )
						{
							DEVPRINTF( "CBarterLevel::InitLevel() : Could not init slot '%d' item inst '%s'", nSlot, pszItemName );
							goto _ExitWithError;
						}
						if (nSlot==AMMO_SLOT)
						{
							if( CCollectable::IsPrimaryWeaponType(pSaleChain->m_paItemInsts[nItemInst].m_pCollectableType) )
							{
								pSaleChain->m_paItemInsts[nItemInst].SetAmmo(TRUE);
							}
							else
							{
								pSaleChain->m_paItemInsts[nItemInst].SetAmmo(FALSE);
							}
						}
						
						if( !pSaleChain->m_paItemInsts[nItemInst].m_poItem->LoadSharedResources())
						{
							DEVPRINTF( "CBarterLevel::InitLevel() : Could not load shared resources for item '%s'", pszItemName );
							goto _ExitWithError;
						}
					}
				}
				for( ; nSalesChain < pSlot->m_nSaleChains; nSalesChain++ ) 
				{
					pSaleChain = &pSlot->m_paSaleChains[nSalesChain];
					pSaleChain->m_paItemInsts = fnew CBarterItemInst[1];
					pSaleChain->m_nNumItems = 1;
				}
			}
			
			// Fill in the unspecified BarterItemInsts with the possible candidates off the CBarterItem array...
			BOOL		bItemIsInListAlready;
			u32 nSalesChainInsertAt = pSlot->m_nSpecSaleChains;
			for (u32 nItem = 0; nItem < CBarterItem::m_aNumItems[nSlot]; nItem++)
			{
				bItemIsInListAlready = FALSE;
				// search to see if we've already got this item in the offer list
				for(u32 nSalesChain=0; nSalesChain < pSlot->m_nSpecSaleChains; nSalesChain++ )
				{
					pSaleChain = &pSlot->m_paSaleChains[nSalesChain];
					if (pSaleChain->m_paItemInsts[0].m_poItem->m_eCollectableType == CBarterItem::m_apaItems[nSlot][nItem].m_eCollectableType)
					{
						bItemIsInListAlready = TRUE;
						break;
					}
				}

#if FANG_WINGC
				if (bItemIsInListAlready == FALSE && nSalesChainInsertAt < pSlot->m_nSaleChains && pSlot->m_paSaleChains)
#else
				if (bItemIsInListAlready == FALSE)
#endif
				{
					pSaleChain = &pSlot->m_paSaleChains[nSalesChainInsertAt];
					pSaleChain->m_paItemInsts[0].Init(&CBarterItem::m_apaItems[nSlot][nItem],FALSE,BARTER_INFINITE,BARTER_DEFAULT_PRICE);
					if( (nSlot==AMMO_SLOT) && CCollectable::IsPrimaryWeaponType(pSaleChain->m_paItemInsts[0].m_pCollectableType) )
					{
						pSaleChain->m_paItemInsts[0].SetAmmo(TRUE);
					}
					else
					{
						pSaleChain->m_paItemInsts[0].SetAmmo(FALSE);
					}
					nSalesChainInsertAt++;
					
					// when and if we're automatically offering an unspecified type, we need to give Nate a chance
					// to make a meshpool for it...
					if ( (pSaleChain->m_paItemInsts[0].m_poItem->m_eCollectableType == COLLECTABLE_PUP_ENERGY) ||
						 (pSaleChain->m_paItemInsts[0].m_poItem->m_eCollectableType == COLLECTABLE_WEAPON_CORING_CHARGE))
					{
						pSaleChain->m_paItemInsts[0].m_poItem->LoadSharedResources();
					}
				}
			}
#if FANG_WINGC
			// only the chains that hold an item
			if( pSlot->m_paSaleChains && nSalesChainInsertAt < pSlot->m_nSaleChains ) {
				pSlot->m_nSaleChains = nSalesChainInsertAt;
			}
#endif
		}
	}
	// load all the generic bot talk responses
	for(u32 nResponse=0; nResponse < CBarterItem::m_nNumGenericResponses; nResponse++ ) 
	{
		if( !CBarterItem::m_paGenericResponses[nResponse].Load() ) 
		{
			DEVPRINTF( "CBarterLevel::InitLevel() : Could not load the generic bot talk, '%s'.\n", CBarterItem::m_paGenericResponses[nResponse].m_pszBarterFilename );
			goto _ExitWithError;
		}
	}
	ftext_SetToDefaults(&m_oTextArea);
	m_oTextArea.fNumberOfLines = 1.0f;
	m_oTextArea.fLineSpacing = 0.008f;
	m_oTextArea.ohFont = '1';
	m_oTextArea.fUpperLeftX = 0.155f;
	m_oTextArea.fLowerRightX = 0.80f;
	m_oTextArea.fUpperLeftY = 0.656f;
	m_oTextArea.fLowerRightY =0.691f;
	m_oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	m_oTextArea.oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.4f );
	m_oTextArea.oColorBorder.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	m_oTextArea.fBorderThicknessX = 0.0f;
	m_oTextArea.fBorderThicknessY = 0.0f;
	m_oTextArea.bVisible = FALSE;
	m_hTextDisplay = ftext_Create(&m_oTextArea);

	FMesh_t		*pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, "gp_ieuk2");
	if (!pMesh)
	{
		DEVPRINTF( "CBarterLevel::InitLevel() : Could not find mesh '%s'\n", "gp_ieuk2");
		goto _ExitWithError;
	}
	MeshInit.pMesh		= pMesh;

	m_pAmmoBoxMesh = fnew CFWorldMesh;
	if (!m_pAmmoBoxMesh)
	{
		DEVPRINTF( "CBarterLevel::InitLevel() : Could not allocate memory for ammobox.\n" );		
		goto _ExitWithError;
	}
	
	m_pAmmoBoxMesh->Init( &MeshInit );

	m_pAmmoBoxMesh->m_nUser = MESHTYPES_UNKNOWN;
	m_pAmmoBoxMesh->m_pUser = NULL;
	m_pAmmoBoxMesh->SetUserTypeBits( 0 );
	m_pAmmoBoxMesh->SetCollisionFlag( FALSE );
	m_pAmmoBoxMesh->SetLineOfSightFlag( FALSE );
	m_pAmmoBoxMesh->UpdateTracker();
	m_pAmmoBoxMesh->RemoveFromWorld();

	fmem_ReleaseFrame( hMemFrame );

	return TRUE;

_ExitWithError:
	UninitLevel();
	fres_ReleaseFrame( hResFrame );
	fmem_ReleaseFrame( hMemFrame );

	return FALSE;
}

void CBarterLevel::UninitLevel() 
{
	u32 nTable, nSlot, nChain;
	CBarterSlot *pSlot;
	CBarterSaleChains *pSaleChain;

	if( m_paTables ) 
	{
		// walk the tables
		for( nTable=0; nTable < m_nNumTables; nTable++ ) 
		{
			// walk the slots
			for( nSlot=0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++ ) 
			{
				pSlot = &m_paTables[nTable].m_aSlots[nSlot];

				if( pSlot->m_paSaleChains ) 
				{
					// walk the sales chains
					for( nChain=0; nChain < pSlot->m_nSaleChains; nChain++ ) 
					{
						pSaleChain = &pSlot->m_paSaleChains[nChain];
						
						if( pSaleChain->m_paItemInsts ) 
						{
							fdelete_array( pSaleChain->m_paItemInsts );
							pSaleChain->m_paItemInsts = NULL;
						}
					}

					fdelete_array( pSlot->m_paSaleChains );
					pSlot->m_paSaleChains = NULL;
				}
			}
		}
		fdelete_array( m_paTables );
		m_paTables = NULL;
	}
	m_pCurrentTable = NULL;
}

void CBarterLevel::Work() {
	
	if( !m_pCurrentTable ) {
		return;
	}
	u32 i;
	CBarterItemInst *pItem;
	for( i=0; i < BARTER_TYPES_NUM_SLOTS; i++ ) {
		pItem = GetSlotItemInst( i );
		if( pItem ) {
			pItem->Work();
		}
	}
}

u32 CBarterLevel::GetNumTables() {
	return m_nNumTables;
}

CBarterTable *CBarterLevel::GetTable( u32 nIndex ) {
	if( !m_paTables || (nIndex >= m_nNumTables) ) {
		return NULL;
	}
	return &m_paTables[nIndex];
}

CBarterTable *CBarterLevel::GetTable( cchar *pszName ) {
	if( !m_paTables ) {
		return NULL;
	}
	u32 i;
	for( i=0; i < m_nNumTables; i++ ) {
		if( fclib_stricmp( m_paTables[i].m_pszName, pszName ) == 0 ) {
			return &m_paTables[i];
		}
	}
	return NULL;
}

void CBarterLevel::SetActiveTable( CBarterTable *pActive ) {

	FASSERT( m_paTables );
	
	if( pActive == m_pCurrentTable ) {
		// already the active table
		return;
	}
	// we should never be directly switching between 2 different tables, only NULL -> Active & Active -> NULL
	FASSERT( (m_pCurrentTable && !pActive) || (!m_pCurrentTable && pActive) );
	
	// set the current table ptr
	m_pCurrentTable = pActive;
}

CBarterTable *CBarterLevel::GetActiveTable() {
	return m_pCurrentTable;
}

void CBarterLevel::SelectStartingItems( CBot *pPlayerBot ) 
{
	FASSERT( pPlayerBot );

	if( !m_pCurrentTable ) 
	{
		return;
	}
	
	FASSERT(pPlayerBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
	_UpdateItemsForSaleBasedOn((CBotGlitch*)pPlayerBot);

	for(s32 nSlot=0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++ ) 
	{
		if( m_pCurrentTable->m_aSlots[nSlot].m_paSaleChains ) 
		{
			// select the sales chain
			m_pCurrentTable->m_aSlots[nSlot].SetChainIndex(fmath_RandomChoice( m_pCurrentTable->m_aSlots[nSlot].m_nSaleChains ));
			m_pCurrentTable->m_aSlots[nSlot].m_paSaleChains[m_pCurrentTable->m_aSlots[nSlot].GetChainIndex()].m_nItemIndex = 0;
		}
	}
}

void CBarterLevel::_UpdateItemsForSaleBasedOn(CBotGlitch* pPlayerBot)
{
	FASSERT( pPlayerBot );
	FASSERT( pPlayerBot->m_pInventory );

	if( !m_pCurrentTable ) 
	{
		return;
	}
	
	CBarterSlot* pSlot; 
	CBarterItemInst* pItem; 

	for(s32 nSlot=0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++ )
	switch(nSlot)
	{
	case 0:	// Slot 0: Items
		// If glitch's is not fully healthy, offer health...
		// all other items need to be spec'ed in order to be offered
		pSlot = &m_pCurrentTable->m_aSlots[nSlot];
		for(u32 nSalesChain=0; nSalesChain < pSlot->m_nSaleChains; nSalesChain++ )
		{
			pItem = &pSlot->m_paSaleChains[nSalesChain].m_paItemInsts[0];
#if FANG_WINGC
			// Legacy EUK-only catalog entries have no collectable enum in this source revision.
			// UNKNOWN would otherwise resolve to an unrelated custom quest part (goffhead).
			if( pItem->m_poItem->m_eCollectableType == COLLECTABLE_UNKNOWN ) {
				pItem->SetNeeded(FALSE);
				continue;
			}
#endif
			if (pItem->m_poItem->m_eCollectableType == COLLECTABLE_BATTERY)
			{
				s8 nBatteriesToSell = (pItem->GetQuantity() - pPlayerBot->m_pInventory->m_uNumBatteries);
				pItem->SetNeeded((nBatteriesToSell > 0) && CCollectable::PlayerNeeds(pPlayerBot,pItem->m_poItem->m_eCollectableType,FALSE));
			}
			else
			{
				pItem->SetNeeded(CCollectable::PlayerNeeds(pPlayerBot,pItem->m_poItem->m_eCollectableType,FALSE));
			}
			if (pItem->m_poItem->m_eCollectableType == COLLECTABLE_PUP_ENERGY)
				pItem->SetAutoOffered(TRUE);
		}
		break;

	case 1:// Slot 1: Ammunition
		// If glitch has instance of the weapon, offer amunition
		// IF the weapon is NOT fully loaded
		// all other items need to be spec'ed in order to be offered
		pSlot = &m_pCurrentTable->m_aSlots[nSlot];
		for(u32 nSalesChain=0; nSalesChain < pSlot->m_nSaleChains; nSalesChain++ )
		{
			pItem = &pSlot->m_paSaleChains[nSalesChain].m_paItemInsts[0];
#if FANG_WINGC
			// Legacy EUK-only catalog entries have no collectable enum in this source revision.
			// UNKNOWN would otherwise resolve to an unrelated custom quest part (goffhead).
			if( pItem->m_poItem->m_eCollectableType == COLLECTABLE_UNKNOWN ) {
				pItem->SetNeeded(FALSE);
				continue;
			}
#endif
			pItem->SetNeeded(CCollectable::PlayerNeeds(pPlayerBot,pItem->m_poItem->m_eCollectableType,pItem->IsAmmo()));
			if (pItem->IsNeeded() && CCollectable::IsPrimaryWeaponType(pItem->m_pCollectableType))
				pItem->SetAutoOffered(TRUE);
		}
		break;

	case 2:	// Slot 2: Weapons
		// If glitch has room for coring charges, offer them
		// all other weapons / weapon upgrades must be spec'ed
		// If glitch's is not fully healthy, offer health...
		// all other items need to be spec'ed in order to be offered
		pSlot = &m_pCurrentTable->m_aSlots[nSlot];
		for(u32 nSalesChain=0; nSalesChain < pSlot->m_nSaleChains; nSalesChain++ )
		{
			pItem = &pSlot->m_paSaleChains[nSalesChain].m_paItemInsts[0];
#if FANG_WINGC
			// Legacy EUK-only catalog entries have no collectable enum in this source revision.
			// UNKNOWN would otherwise resolve to an unrelated custom quest part (goffhead).
			if( pItem->m_poItem->m_eCollectableType == COLLECTABLE_UNKNOWN ) {
				pItem->SetNeeded(FALSE);
				continue;
			}
#endif
			pItem->SetNeeded(CCollectable::PlayerNeeds(pPlayerBot,pItem->m_poItem->m_eCollectableType,FALSE));
			if (pItem->m_poItem->m_eCollectableType == COLLECTABLE_WEAPON_CORING_CHARGE)
				pItem->SetAutoOffered(TRUE);
		}
		break;
	}
}

// NOTE: SLOT INDICES ARE NEVER REMAPPED AT THIS LEVEL, REGARDLESS OF UPSALE MODE OR NOT!!!!
void CBarterLevel::SetCurrentSlot( u32 nSlotIndex ) {	

	FASSERT( nSlotIndex < BARTER_TYPES_NUM_SLOTS );

	if( m_pCurrentTable ) {

		CBarterItemInst *pItem;
		// unselect the old item
		pItem = GetCurrentItemInst();
		if( pItem ) {
			pItem->SetSelected( FALSE );
		}
		// select the new item
		m_pCurrentTable->m_nCurrentSlot = nSlotIndex;
		pItem = GetCurrentItemInst();
		if( pItem ) {
			pItem->SetSelected( TRUE );
		}
	}
}

// Will select the best starting slot, based on the current slot layout.
// Will try to maintain previous slot first, then trys to:
// Will try to select the middle most non-empty slot and will return the index;
//
// NOTE: SLOT INDICES ARE NEVER REMAPPED AT THIS LEVEL, REGARDLESS OF UPSALE MODE OR NOT!!!!
s32 CBarterLevel::SetStartingSlotIndex() {
	u32 i;

	FASSERT( m_pCurrentTable );
	u32 nCurSlot = GetCurrentSlot();
    if ( (nCurSlot >= 0) && (nCurSlot < BARTER_TYPES_NUM_SLOTS) && (m_pCurrentTable->m_aSlots[nCurSlot].GetChainIndex() != BARTER_SLOT_UNUSED_SLOT ) )
	{
		SetCurrentSlot( nCurSlot );
		return nCurSlot;
	}

	if( m_pCurrentTable->m_aSlots[1].m_paSaleChains &&
		m_pCurrentTable->m_aSlots[1].GetChainIndex() != BARTER_SLOT_UNUSED_SLOT ) {
		// we can set the middle slot as the starting slot
		SetCurrentSlot( 1 );
		return 1;
	}

	// just find an non-empty slot to make current
	for( i=0; i < BARTER_TYPES_NUM_SLOTS; i++ ) {
		if( m_pCurrentTable->m_aSlots[i].m_paSaleChains &&
			m_pCurrentTable->m_aSlots[i].GetChainIndex() != BARTER_SLOT_UNUSED_SLOT ) {
			// we can set the middle slot as the starting slot
			SetCurrentSlot( i );
			return i;
		}
	}
//	MRS: Allowing null tables, as exit condition
//	FASSERT_NOW;// SHOULD NEVER GET HERE, THIS MEANS A TABLE WAS COMPLETELY EMPTY

//	SetCurrentSlot( 0 );
	return -1;
}

// NOTE: SLOT INDICES ARE NEVER REMAPPED AT THIS LEVEL, REGARDLESS OF UPSALE MODE OR NOT!!!!
u32 CBarterLevel::GetCurrentSlot() {
	
	if( m_pCurrentTable ) {
		return m_pCurrentTable->m_nCurrentSlot;
	}

	return 0;
}

void CBarterLevel::SetCurrentSalesChain( s32 nSalesChainIndex )
{
	if( m_pCurrentTable ) 
	{
		CBarterSlot* pSlot = &m_pCurrentTable->m_aSlots[m_pCurrentTable->m_nCurrentSlot];
		
		BOOL bSearchUp =  (nSalesChainIndex >= pSlot->GetChainIndex());

		if (nSalesChainIndex < 0) // wrap bottom
			nSalesChainIndex = pSlot->m_nSaleChains-1;

		if (nSalesChainIndex > (s32)pSlot->m_nSaleChains-1) // wrap top
			nSalesChainIndex = 0;
		
		
		if (nSalesChainIndex != pSlot->GetChainIndex())
		{
			CBarterSaleChains *pSaleChain = NULL;
			// reset the previous sales chain
			if( pSlot->GetChainIndex() != BARTER_SLOT_UNUSED_SLOT ) 
			{
				pSaleChain = &pSlot->m_paSaleChains[ pSlot->GetChainIndex() ];
				pSaleChain->m_paItemInsts[pSaleChain->m_nItemIndex].SetSelected( FALSE );
				pSaleChain->m_nItemIndex = 0;
			}

			// pick a new sale chain for this slot
			pSlot->SetChainIndex(nSalesChainIndex,bSearchUp);
			if ( pSlot->GetChainIndex() != BARTER_SLOT_UNUSED_SLOT)
			{
				pSaleChain = &pSlot->m_paSaleChains[ pSlot->GetChainIndex() ];
				pSaleChain->m_nItemIndex = 0;
				pSaleChain->m_paItemInsts[pSaleChain->m_nItemIndex].SetSelected( TRUE );
			}
		}
	}
}

u32  CBarterLevel::GetCurrentSalesChainIndex(void)
{
	if( m_pCurrentTable ) 
	{
		return m_pCurrentTable->m_aSlots[m_pCurrentTable->m_nCurrentSlot].GetChainIndex();
	}
	return 0;
}


// returns TRUE if the item was purchased by the player, FALSE if not
BOOL CBarterLevel::PurchaseCurrentItem( CBot *pPlayerBot ) {

	FASSERT( pPlayerBot );

	CBarterItemInst *pItem = GetCurrentItemInst();
	if( !pItem ) {
		return FALSE;
	}
	if( !pItem->CanPurchase( pPlayerBot ) ) {
		return FALSE;
	}
	pItem->Purchase( pPlayerBot );

	return TRUE;
}

// sets a new table up.
// returns TRUE if we are now in upsale mode, FALSE if not
BOOL CBarterLevel::SelectNewItems( CBot *pPlayerBot, BOOL bPlayerJustBoughtCurrentItem ) {

	FASSERT( pPlayerBot );
	FASSERT( m_pCurrentTable );

	// pick new sale chains for the nonselected slots and 
	// pick either the next item in the selected chain or a new sales chain
	u32 i, nIndex;
	BOOL bUpSale = FALSE;
	CBarterSlot *pSlot;
	CBarterSaleChains *pSaleChain;

	_UpdateItemsForSaleBasedOn((CBotGlitch*)pPlayerBot);

	for( i=0; i < BARTER_TYPES_NUM_SLOTS; i++ ) {
		pSlot = &m_pCurrentTable->m_aSlots[i];

		if( !pSlot->m_paSaleChains ) {
			// empty
			continue;
		}

		if( i == m_pCurrentTable->m_nCurrentSlot ) {
			// this is the current slot
			FASSERT( pSlot->GetChainIndex() != BARTER_SLOT_UNUSED_SLOT );

			pSaleChain = &pSlot->m_paSaleChains[ pSlot->GetChainIndex() ];

			if( bPlayerJustBoughtCurrentItem ) 
			{
				nIndex = pSaleChain->m_nItemIndex+1;
				if( nIndex < pSaleChain->m_nNumItems ) 
				{
					BOOL bAmmoNeeded = FALSE;
					BOOL bWeaponNeeded = CCollectable::PlayerNeeds((CBotGlitch*)pPlayerBot,pSaleChain->m_paItemInsts[nIndex].m_poItem->m_eCollectableType,FALSE);
					if (!bWeaponNeeded)
					{
						bAmmoNeeded = CCollectable::PlayerNeeds((CBotGlitch*)pPlayerBot,pSaleChain->m_paItemInsts[nIndex].m_poItem->m_eCollectableType,TRUE);
					}
					pSaleChain->m_paItemInsts[nIndex].SetAmmo(bAmmoNeeded);
					// offer the upgrade
					if (bAmmoNeeded || bWeaponNeeded)
					{
						pSaleChain->m_paItemInsts[nIndex-1].SetSelected( FALSE );
						pSaleChain->m_paItemInsts[nIndex].SetSelected( TRUE );
						pSaleChain->m_nItemIndex = nIndex;
						bUpSale = TRUE;
						continue;
					}
				}
			}			
		}

		// reset the previous sales chain
		if( pSlot->GetChainIndex() != BARTER_SLOT_UNUSED_SLOT ) {
			pSaleChain = &pSlot->m_paSaleChains[ pSlot->GetChainIndex() ];
			pSaleChain->m_paItemInsts[pSaleChain->m_nItemIndex].SetSelected( FALSE );
			pSaleChain->m_nItemIndex = 0;
		}

		// just pick a new sale chain for this slot
		pSlot->SetChainIndex(fmath_RandomChoice( pSlot->m_nSaleChains ));
		if (pSlot->GetChainIndex() != BARTER_SLOT_UNUSED_SLOT)
		{
			pSlot->m_paSaleChains[pSlot->GetChainIndex()].m_nItemIndex = 0;
		}
	}

	return bUpSale;
}

BOOL CBarterLevel::IsInUpSaleMode() {

	if( HowDeepIsCurrentItemInSaleChain() > 0 ) {
		return TRUE;
	}
	// the current slot is either empty or at the first item in the chain
	return FALSE;
}

// returns the index of the current item in its sale chain,
// 0 means it is the first item in the chain, 1 the 2nd...
u32 CBarterLevel::HowDeepIsCurrentItemInSaleChain() {

	if( !m_pCurrentTable ) {
		return 0;
	}
	CBarterSlot *pSlot = &m_pCurrentTable->m_aSlots[m_pCurrentTable->m_nCurrentSlot];

	if( !pSlot->m_paSaleChains || 
		pSlot->GetChainIndex() == BARTER_SLOT_UNUSED_SLOT ) {
		// empty slot
		return 0;
	}

	return pSlot->m_paSaleChains[ pSlot->GetChainIndex() ].m_nItemIndex;
}

CBarterItemInst *CBarterLevel::GetCurrentItemInst() {

	if( !m_pCurrentTable ) {
		return NULL;
	}
	return GetSlotItemInst( m_pCurrentTable->m_nCurrentSlot );
}

CBarterItemInst *CBarterLevel::GetSlotItemInst( u32 nSlotIndex ) {

	if( m_pCurrentTable ) {
		CBarterSlot *pSlot = &m_pCurrentTable->m_aSlots[nSlotIndex];
		if( pSlot->m_paSaleChains &&
			pSlot->GetChainIndex() != BARTER_SLOT_UNUSED_SLOT ) {
			// this is not an empty slot
			CBarterSaleChains *pSalesChain = &pSlot->m_paSaleChains[ pSlot->GetChainIndex() ];
			
			return &pSalesChain->m_paItemInsts[ pSalesChain->m_nItemIndex ];
		}
	}
	return NULL;
}

u32 CBarterLevel::GetSlotItemCount( u32 nSlotIndex ) 
{
	u32 nItemCount = 0;
	if( m_pCurrentTable ) 
	{
		CBarterSlot *pSlot = &m_pCurrentTable->m_aSlots[nSlotIndex];
		if( pSlot->m_paSaleChains && pSlot->GetChainIndex() != BARTER_SLOT_UNUSED_SLOT ) 
		{
			for (u32 uChainIndex = 0; uChainIndex < pSlot->m_nSaleChains; uChainIndex++)
			{
				CBarterItemInst* pBIIToTest = &pSlot->m_paSaleChains[uChainIndex].m_paItemInsts[0];
				FASSERT(pBIIToTest);
				if(pBIIToTest->ShouldBeOffered())
					nItemCount++;
			}
		}
	}
	return nItemCount;
}

void CBarterSlot::SetChainIndex( s32 nNewChainIndex, BOOL bSearchUp )
{
	FMATH_CLAMP(nNewChainIndex, BARTER_SLOT_UNUSED_SLOT,(s32)m_nSaleChains-1);
	if (nNewChainIndex == BARTER_SLOT_UNUSED_SLOT)
	{
		m_nChainIndex = nNewChainIndex;
		return;
	}

	CBarterItemInst* pBIIToTest = &m_paSaleChains[nNewChainIndex].m_paItemInsts[0];
	u32 nTestIndex;

	if (pBIIToTest->ShouldBeOffered())
	{
		m_nChainIndex = nNewChainIndex;
		return;
	}
	else if (!bSearchUp) // decrementing search
	{
		if (m_nChainIndex == BARTER_SLOT_UNUSED_SLOT)
			m_nChainIndex = nNewChainIndex;
		
		nTestIndex = m_nChainIndex-1;
		if (nTestIndex > m_nSaleChains-1) // wraps around to max
			nTestIndex = m_nSaleChains-1;
		while (nTestIndex != m_nChainIndex)
		{
			pBIIToTest = &m_paSaleChains[nTestIndex].m_paItemInsts[0];
			if (pBIIToTest->ShouldBeOffered())
				break;
			else
				nTestIndex--; // wraps unsignedly, so CLAMPMAX will catch
			if (nTestIndex > m_nSaleChains-1) // wraps around to max
				nTestIndex = m_nSaleChains-1;
		}

		if (m_paSaleChains[nTestIndex].m_paItemInsts[0].ShouldBeOffered())
		{
			m_nChainIndex = nTestIndex;
		}
		else
		{
			// get here, means didn't find any good guys
			m_nChainIndex = BARTER_SLOT_UNUSED_SLOT;
		}
	}
	else // incrementing search
	{
		if (m_nChainIndex == BARTER_SLOT_UNUSED_SLOT)
			m_nChainIndex = nNewChainIndex;
		
		nTestIndex = m_nChainIndex+1;
		if (nTestIndex > m_nSaleChains-1) // wraps around to zero
			nTestIndex = 0;
		while (nTestIndex != m_nChainIndex)
		{
			pBIIToTest = &m_paSaleChains[nTestIndex].m_paItemInsts[0];
			if (pBIIToTest->ShouldBeOffered())
				break;
			else
				nTestIndex++; // wraps unsignedly, so CLAMPMAX will catch
			if (nTestIndex > m_nSaleChains-1) // wraps around to zero
				nTestIndex = 0;
		}

		if (m_paSaleChains[nTestIndex].m_paItemInsts[0].ShouldBeOffered())
		{
			m_nChainIndex = nTestIndex;
		}
		else
		{
			// get here, means didn't find any good guys
			m_nChainIndex = BARTER_SLOT_UNUSED_SLOT;
		}
	}
}

void CBarterTable::CheckPointSave(void)
{
	CBarterSlot* pSlot = NULL;
	for (s32 nSlot = 0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++)
	{
		pSlot = &m_aSlots[nSlot];
		pSlot->CheckPointSave();
	}
}

void CBarterTable::CheckPointRestore(void)
{
	CBarterSlot* pSlot = NULL;
	for (s32 nSlot = 0; nSlot < BARTER_TYPES_NUM_SLOTS; nSlot++)
	{
		pSlot = &m_aSlots[nSlot];
		pSlot->CheckPointRestore();
	}
}

void CBarterSlot::CheckPointSave(void)
{
	CBarterSaleChains* pSalesChain = NULL;
	CBarterItemInst* pBarterItemInst = NULL;
	// since only spec chains have good quantity's they're the only ones we need to save
	for (u32 nSalesChain = 0; nSalesChain < m_nSpecSaleChains; nSalesChain++)
	{
		pSalesChain = &m_paSaleChains[nSalesChain];
		for (u32 nItem = 0; nItem < pSalesChain->m_nNumItems ; nItem++)
		{
			pBarterItemInst = &pSalesChain->m_paItemInsts[nItem];
			CFCheckPoint::SaveData(pBarterItemInst->GetQuantity());
		}
	}
}

void CBarterSlot::CheckPointRestore(void)
{
	CBarterSaleChains* pSalesChain = NULL;
	CBarterItemInst* pBarterItemInst = NULL;
	s8 nItemQuantity=0;
	// since only spec chains have good quantity's they're the only ones we need to restore
	for (u32 nSalesChain = 0; nSalesChain < m_nSpecSaleChains; nSalesChain++)
	{
		pSalesChain = &m_paSaleChains[nSalesChain];
		for (u32 nItem = 0; nItem < pSalesChain->m_nNumItems ; nItem++)
		{
			pBarterItemInst = &pSalesChain->m_paItemInsts[nItem];
			CFCheckPoint::LoadData(nItemQuantity);
			pBarterItemInst->SetQuantity(nItemQuantity);
		}
	}
}
