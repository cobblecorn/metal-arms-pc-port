//////////////////////////////////////////////////////////////////////////////////////
// AIBTATable.cpp - 
//
// Author: Pat MacKellar 
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
// 09/10/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fres.h"
#include "flinklist.h"
#include "fresload.h"
#include "AIBTATable.h"
#include "AIBrain.h"
#include "AIGameUtils.h"
#include "AiMover.h"
#include "fclib.h"
#include "fmath.h"
#include "../TalkSystem2.h"
#include "../gstring.h"

/*  There is full documentation of the event description string format in a word doc on the server.

***_RUN		   - bot decides to run for cover, or hide or panic run
***_NAPJ	   - nap jerk
***_HEAR	   - when they hear something
***_SUS1	   - suspicion level 1
***_SUS2
***_SUS3
***_SUS4
***_GRE1		- greeting level 1
***_GRE2
***_GRE3
***_GRE4
***_OUCH		- take damage
***_TAUN		- taunt
***_LAFF		- laugh
***_SEES		- when they see something for the first time
***_DEAD		- when they die
***_SRCH		- when bot has been attacking, but decides to give up and search for you
***_VICT		- when they win a battle (victor)
***_SWTC		- when they need to press a switch
***_BOSS		- something a recruit says to its leader when leader does something good.
***_GREN		- when a bot sees or throws a grenade, it might say this
***_MOTI		- said to leader when idle
***_IDLE		- played when standing around for a while with nothing to do (usually silent)
***_NOPE		- no means no.
*/


u32 _kDIALOG_DELAY_MIN =4;			//minimum time between dialogs
u32 _kDIALOG_DELAY_MAX =8;			//minimum time between dialogs
f32 _fDialogPlayTimeOut = 0.0f;
f32 _fLastDialogPlay = 0.0f;

u32 _kLAUGH_DELAY_NORM_MIN=2;		//minimum time between laughs
u32 _kLAUGH_DELAY_NORM_MAX=7;		//minimum time between laughs
f32 _fLaughPlayTimeOut = 0.0f;

//some dialogs have punchlines.
//a punch line dialog disables the global laugh timer for some time.
u32 _kDISABLE_LAUGH_DELAY_POSTTAUNT_MIN=1;  //when the laugh delay timer is disabled, it will be for this long
u32 _kDISABLE_LAUGH_DELAY_POSTTAUNT_MAX=5;  
u32 _kMAX_GROUP_LAUGHS =2;  
f32 _fDisableLaughTimeOutTimeOut = 0.0f;
u32 _uGroupLaughCounter = 0;

#define CATEGORY_UNPLAYED 0
#define CATEGORY_PLAYED__ 1

#define _MAX_LOCKGROUP		2
#define _MAX_LOCKSUBGROUP	3

struct BTAUseage
{
	//////////////////////// Configure these fields when adding BTA's to the BTA table
	cchar* pszEvent;
	cchar* pszBTAName;
	u32 uLockGroup;						   //When the first bta is played from this group, set all other bta's in this group, but NOT in this subgroup lock, to have been played (will reset when all in group have been played)
	u32 uLockSubGroup;
	BOOL bHasDialog;					   //There is a minimum time between any two dialogs BTA
	BOOL bHasLaugh;
	BOOL bHasPunchline;
	u32 nInstPoolSize;
	
	
	////////////////////////// RT data fields. BTATAble will mess with these.
	u32 uPlayCount;
	f32 fLastTime;
	BOOL bBadLoad;
	s8 bCategoryPlayTracking;				  //all BTA in any particular category will be played before any is repeated.
	s8 uLockGroupPlayTracking;				  
};

struct BTAUseageCSV
{
	cchar* pszEvent;
	cchar* pszBTAName;
	u32 uLockGroup;						//When the first bta is played from this group, set all other bta's in this group, but NOT in this subgroup lock, to have been played (will reset when all in group have been played)
	u32 uLockSubGroup;
	cchar* pszHasDialog;				//There is a global minimum time between any two dialogs BTA
	cchar* pszHasLaugh;					//Thre is a global minimum time between any two dialogs BTA
	cchar* pszHasPunchline;
	u32 nInstPoolSize;
};

struct BTAGroup {
	cchar *m_pszMeshName;

	u32 m_uBTAUsageTableSize;
	BTAUseage *m_paBTAUsageTable;

	FLink_t m_Link;
};

const FGameData_TableEntry_t _aBTADataVocab[] = {
	// pszEvent
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszBTAName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// uLockGroup
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// uLockSubGroup
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// bHasDialog
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// bHasLaugh
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// bHasPunchline
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// nInstPoolSize
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

u32 _auLockGroupActiveSubGroup[_MAX_LOCKGROUP];
FLinkRoot_t _BTALinkRoot;

FINLINE BOOL _IsYes( cchar *pszStr ) {
	return fclib_stricmp( pszStr, "yes" ) == 0;
}

#if FANG_PLATFORM_GC
cchar* _pszSoundBanksLabel = "soundbanks_gc";
#else
cchar* _pszSoundBanksLabel = "soundbanks_xb";
#endif


BOOL _ProcessSoundTables( FGameDataFileHandle_t hFile ) {
	u32 uNum;
	cchar *pszStr;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;

	hTable = fgamedata_GetFirstTableHandle( hFile, _pszSoundBanksLabel );

	// It is okay to have no sounds
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE )
	{
		hTable = fgamedata_GetFirstTableHandle( hFile, "soundbanks" );
		
		if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE )
		{
			return TRUE;
		}
	}



	uNum = fgamedata_GetNumFields( hTable );

	for( u32 i = 0; i < uNum; ++i ) {
		pszStr = (cchar *) fgamedata_GetPtrToFieldData( hTable, i, nDataType );

		if( pszStr == NULL || nDataType != FGAMEDATA_VAR_TYPE_STRING ) {
			DEVPRINTF( "AIBTA_LoadBTAGroup(): Mesh name is not a string, ignoring table.\n" );
			goto _ExitWithError;
		}			

		// Load the sound banks
		if( !fresload_Load( FSNDFX_RESTYPE, pszStr ) ) {
			DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessSoundTables(): Could not load sound effect bank '%s'.\n", pszStr );
		}
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}

BOOL _ProcessBotTalks( FGameDataFileHandle_t hFile, BTAGroup *pBTAGroup ) {
	FGameDataTableHandle_t hTable;
	u32 nTableFieldCount, nArrayElementCount, nStructElementCount;
	u32 nTableEntryCount, uFieldIndex = 0;		//CPS 4.7.03

	hTable = fgamedata_GetFirstTableHandle( hFile, "bottalks" );

	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessBotTalks(): Failed to find 'bottalks' handle, ignoring table.\n" );
		goto _ExitWithError;
	}

	nStructElementCount = sizeof( _aBTADataVocab ) / sizeof( FGameData_TableEntry_t );
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;

	nTableFieldCount = fgamedata_GetNumFields( hTable );
	if( (nTableFieldCount % nStructElementCount) ) {
		DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessBotTalks(): Invalid number of fields in 'bottalks' entry. Must be a multiple of %u.\n", nStructElementCount );
		goto _ExitWithError;
	}

	nArrayElementCount = nTableFieldCount / nStructElementCount;

	pBTAGroup->m_paBTAUsageTable = (BTAUseage *) fres_Alloc( sizeof( BTAUseage ) * nArrayElementCount );

	if( pBTAGroup->m_paBTAUsageTable == NULL ) {
		DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessBotTalks(): Not enough memory for BTA usage tables.\n" );
		goto _ExitWithError;
	}

	pBTAGroup->m_uBTAUsageTableSize = nArrayElementCount;

	BTAUseageCSV Usage;

//CPS 4.7.03	u32 nTableEntryCount, uFieldIndex = 0;
	uFieldIndex = 0;		//CPS 4.7.03

	for( nTableEntryCount=0; _aBTADataVocab[nTableEntryCount].GetDataType() != FGAMEDATA_VAR_TYPE_COUNT; ++nTableEntryCount ) {}

	for( u32 i = 0; i < nArrayElementCount; ++i ) {
		if( !fgamedata_GetTableData( hTable, _aBTADataVocab, &Usage, sizeof( BTAUseageCSV ), uFieldIndex ) ) {
			DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessBotTalks(): Trouble parsing usage table.\n" );
			goto _ExitWithError;
		}

		if( Usage.pszEvent == NULL ) {
			DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessBotTalks(): Null event name.\n" );
			goto _ExitWithError;
		}

		if( Usage.pszBTAName == NULL ) {
			DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessBotTalks(): Null BTA name.\n" );
			goto _ExitWithError;
		}

		if( Usage.uLockGroup >= _MAX_LOCKGROUP ) {
			DEVPRINTF( "AIBTA_LoadBTAGroup::_ProcessBotTalks(): Invalid lock group '%d'.  Forcing group to 0\n", Usage.uLockGroup );
			Usage.uLockGroup = 0;
		}

		pBTAGroup->m_paBTAUsageTable[i].pszEvent		= Usage.pszEvent;
		pBTAGroup->m_paBTAUsageTable[i].pszBTAName		= Usage.pszBTAName;
		pBTAGroup->m_paBTAUsageTable[i].uLockGroup		= Usage.uLockGroup;
		pBTAGroup->m_paBTAUsageTable[i].uLockSubGroup	= Usage.uLockSubGroup;
		pBTAGroup->m_paBTAUsageTable[i].bHasDialog		= _IsYes( Usage.pszHasDialog );
		pBTAGroup->m_paBTAUsageTable[i].bHasLaugh		= _IsYes( Usage.pszHasLaugh );
		pBTAGroup->m_paBTAUsageTable[i].bHasPunchline	= _IsYes( Usage.pszHasPunchline );
		pBTAGroup->m_paBTAUsageTable[i].nInstPoolSize	= Usage.nInstPoolSize;

		pBTAGroup->m_paBTAUsageTable[i].uPlayCount				= 0;
		pBTAGroup->m_paBTAUsageTable[i].fLastTime				= 0.0f;
		pBTAGroup->m_paBTAUsageTable[i].bCategoryPlayTracking	= CATEGORY_UNPLAYED;
		pBTAGroup->m_paBTAUsageTable[i].uLockGroupPlayTracking	= 0;

		CTalkSystem2::BTIPool_Init(pBTAGroup->m_paBTAUsageTable[i].pszBTAName, pBTAGroup->m_paBTAUsageTable[i].nInstPoolSize);

		uFieldIndex += nTableEntryCount;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}

void AIBTA_InitSystem(void)
{
	u32 i;
	_fDialogPlayTimeOut = 0.0f;
	_fLaughPlayTimeOut = 0.0f;
	_fDisableLaughTimeOutTimeOut = 0.0f;

	for (i = 0; i < _MAX_LOCKGROUP; i++)
	{
		_auLockGroupActiveSubGroup[i] = 0;
	}

	flinklist_InitRoot( &_BTALinkRoot, FANG_OFFSETOF( BTAGroup, m_Link ) );
}

void AIBTA_UninitSystem(void) 
{
	flinklist_InitRoot( &_BTALinkRoot, FANG_OFFSETOF( BTAGroup, m_Link ) );
}

void AIBTA_ResetGlobalTimers(void)
{
 	_fDialogPlayTimeOut = 0.0f;
	_fLaughPlayTimeOut = 0.0f;
	_fDisableLaughTimeOutTimeOut = 0.0f;

}

BOOL MatchUseageStrings(cchar* pszEventString, cchar* pszUseageString, BOOL bRequireDetailMatch)
{
	u32 uLen1 = fclib_strlen(pszEventString);
	u32 uLen2 = fclib_strlen(pszUseageString);
	u32 uMinLen = uLen1;
	u32 i;
	if (uLen2 > uLen1)
	{
		uMinLen = uLen2;
	}

	for (i = 0; i < 8 && i < uMinLen; i++)
	{
		if (pszEventString[i] == '*' ||
			pszUseageString[i] == '*' ||
			(pszEventString[i] == pszUseageString[i]))
		{
			i++;
		}
		else
		{
			break;
		}
	}

	if (!bRequireDetailMatch || uLen1 == 8)
	{
		return i == 8;
	}

	if (uLen1 > 8 && uLen2==uLen1)
	{
		for (i = 8; i < uLen1; i++)
		{
			if (pszEventString[i] == '*' ||
				pszUseageString[i] == '*' ||
				(pszEventString[i] == pszUseageString[i]))
			{
				i++;
			}
		}
		
		if (i==	uLen1)
		{
			return TRUE;
		}
	}

	return TRUE;
}


BOOL MatchCategoryStrings(cchar* pszCatString, cchar* pszBTACatString)
{
	return !fclib_strnicmp(pszCatString+4, pszBTACatString+4, 4);
}


#define MAXNUM_MATCHES 64
cchar* AIBTA_EventToBTAName(cchar* pszEvent, CAIBrain* pBrain)
{
	cchar* pszBTAName = NULL;
	char* pszMeshName = NULL;
	static u8 auSearchBTAGroupIndex[MAXNUM_MATCHES];
	static BTAGroup * apSearchBTAGroup[MAXNUM_MATCHES];
	static u8 auSearchBTAScramble[MAXNUM_MATCHES];

	u32 uNumMatches = 0;
	f32 fNowTime = aiutils_FTotalLoopSecs();
	u32 i, j;
	BTAGroup *pCurrent = (BTAGroup *) flinklist_GetHead( &_BTALinkRoot );

	//
	//   Loop through all tables collecting all BTAs that
	//		a) match the EVENT and 
	//      b) are currently playable
	//
	if (pBrain->GetAIMover()->GetEntity() &&
		(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) &&
		(pszMeshName = ((CBot*) pBrain->GetAIMover()->GetEntity())->m_pWorldMesh->m_pMesh->szName))
	{
		while( pCurrent ) 
		{
			char cMeshCodeDig0 = fclib_tolower(pszMeshName[2]);
			char cMeshCodeDig1 = fclib_tolower(pszMeshName[3]);

			if (cMeshCodeDig0 == fclib_tolower( pCurrent->m_pszMeshName[0] ) &&
				cMeshCodeDig1 == fclib_tolower( pCurrent->m_pszMeshName[1] ))
			{	
				//found a BTAUseage table for this mesh

				for( j = 0; j < pCurrent->m_uBTAUsageTableSize; ++j )
				{
					if ((!pCurrent->m_paBTAUsageTable[j].bHasDialog || _fDialogPlayTimeOut < fNowTime) &&	 //can't play that one right now, it has dialog and a dialog may still be playing
						(!pCurrent->m_paBTAUsageTable[j].bHasLaugh || _fDisableLaughTimeOutTimeOut > fNowTime || _fLaughPlayTimeOut < fNowTime) &&
						MatchCategoryStrings(pszEvent, pCurrent->m_paBTAUsageTable[j].pszEvent) &&
						MatchUseageStrings(pszEvent, pCurrent->m_paBTAUsageTable[j].pszEvent, FALSE))
					{
						auSearchBTAGroupIndex[uNumMatches] = (u8) j;
						apSearchBTAGroup[uNumMatches] = pCurrent;
						auSearchBTAScramble[uNumMatches] = uNumMatches;
						FASSERT( uNumMatches < MAXNUM_MATCHES-1 );
						uNumMatches++;
					}
				}
			}

			pCurrent = (BTAGroup *) flinklist_GetNext( &_BTALinkRoot, pCurrent );
		}
	}


	if (uNumMatches)
	{
		s32 nWinner = -1;
		u32 uPlayCounterReset = 0;

		//scramble the matches
		if (uNumMatches > 1)
		{
			u32 uRandStep = 13+fmath_RandomChoice(uNumMatches);
			u8 tmp;
			for (i = 0; i < uNumMatches;i++)
			{
				j = (i+uRandStep)%uNumMatches;
				tmp = auSearchBTAScramble[i];
				auSearchBTAScramble[i] = auSearchBTAScramble[j];
				auSearchBTAScramble[j] = tmp;
			}
		}

		i = 0;
		while (nWinner == -1 && i < uNumMatches)
		{
			u8 uTestIndex = auSearchBTAGroupIndex[auSearchBTAScramble[i]];
			BTAGroup* pTestGroup = apSearchBTAGroup[auSearchBTAScramble[i]];
			
			if (!pTestGroup->m_paBTAUsageTable[uTestIndex].bCategoryPlayTracking &&
				!(pTestGroup->m_paBTAUsageTable[uTestIndex].uLockGroup != 0 &&
					_auLockGroupActiveSubGroup[pTestGroup->m_paBTAUsageTable[uTestIndex].uLockGroup] != 0 &&
					_auLockGroupActiveSubGroup[pTestGroup->m_paBTAUsageTable[uTestIndex].uLockGroup] != pTestGroup->m_paBTAUsageTable[uTestIndex].uLockSubGroup))
			{
				nWinner = i;
				break;
			}

			i++;

			if (i >= uNumMatches && uPlayCounterReset ==0)
			{
				//reset all playcounters and go through match list again, looking for a candidate
				for (j = 0; j < uNumMatches; j++)
				{
					u8 uTestIndex = auSearchBTAGroupIndex[j];
					BTAGroup* pTestGroup = apSearchBTAGroup[j];
					pTestGroup->m_paBTAUsageTable[uTestIndex].bCategoryPlayTracking = 0;
				}

				uPlayCounterReset++;
				i = 0;
			}
		}

		if (nWinner != -1)
		{	//hit!
			u8 uWinnerIndex = auSearchBTAGroupIndex[auSearchBTAScramble[nWinner]];
			BTAGroup* pWinnerGroup = apSearchBTAGroup[auSearchBTAScramble[nWinner]];

			pszBTAName = pWinnerGroup->m_paBTAUsageTable[uWinnerIndex].pszBTAName;
			
			//track when each bta is
			pWinnerGroup->m_paBTAUsageTable[uWinnerIndex].bCategoryPlayTracking = TRUE;
			
			//reset dialog timer
			if (pWinnerGroup->m_paBTAUsageTable[uWinnerIndex].bHasDialog)
			{
				_fDialogPlayTimeOut	= _kDIALOG_DELAY_MIN + fmath_RandomChoice(_kDIALOG_DELAY_MAX - _kDIALOG_DELAY_MIN) + aiutils_FTotalLoopSecs();

				if (pWinnerGroup->m_paBTAUsageTable[uWinnerIndex].bHasPunchline && fmath_RandomChoice(100) < 50)
				{
					_fDisableLaughTimeOutTimeOut = _kDISABLE_LAUGH_DELAY_POSTTAUNT_MIN + fmath_RandomChoice(_kDISABLE_LAUGH_DELAY_POSTTAUNT_MAX - _kDISABLE_LAUGH_DELAY_POSTTAUNT_MIN) + aiutils_FTotalLoopSecs();
					_uGroupLaughCounter = 0;
				}

				_fLastDialogPlay = aiutils_FTotalLoopSecs();
			}

			//reset laugh timer
			if (pWinnerGroup->m_paBTAUsageTable[uWinnerIndex].bHasLaugh)
			{
				_fLaughPlayTimeOut = _kLAUGH_DELAY_NORM_MIN + fmath_RandomChoice(_kLAUGH_DELAY_NORM_MAX - _kLAUGH_DELAY_NORM_MIN) + aiutils_FTotalLoopSecs();
			}

			
			//deal with Lock groups	(FINDFIX: this lockgroup stuff seems to be jacked, now that there are multiple bots (ie. groups) sharing a lock group
			pCurrent = pWinnerGroup;

			if (pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroup != 0)
			{
				pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroupPlayTracking = TRUE;

				_auLockGroupActiveSubGroup[pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroup] = pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockSubGroup;
			
				u32 uCountNotPlayedInSubGroup = 0;
				for (j = 0; j < pCurrent->m_uBTAUsageTableSize; j++)
				{
					if (j == uWinnerIndex)
					{
						continue;
					}

					if (pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroup == pCurrent->m_paBTAUsageTable[j].uLockGroup)
					{
						if (_auLockGroupActiveSubGroup[pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroup] == pCurrent->m_paBTAUsageTable[j].uLockSubGroup &&
							!pCurrent->m_paBTAUsageTable[j].uLockGroupPlayTracking)
						{
							uCountNotPlayedInSubGroup++;
						}
						else if (_auLockGroupActiveSubGroup[pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroup] != pCurrent->m_paBTAUsageTable[j].uLockSubGroup)
						{
							pCurrent->m_paBTAUsageTable[j].uLockGroupPlayTracking = TRUE;
						}
					}
				}	

				if (!uCountNotPlayedInSubGroup)
				{
					for (j = 0; j < pCurrent->m_uBTAUsageTableSize; j++)
					{  
						if (pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroup == pCurrent->m_paBTAUsageTable[j].uLockGroup)
						{
							pCurrent->m_paBTAUsageTable[j].uLockGroupPlayTracking = FALSE;
						}
					}
					_auLockGroupActiveSubGroup[pCurrent->m_paBTAUsageTable[uWinnerIndex].uLockGroup] = 0;
				}
			}
		}
	}				



	return pszBTAName;
}


BOOL AIBTA_IsBTATaunt(cchar* pszBTAName)
{
	return MatchCategoryStrings(pszBTAName, "****_TAUN");

}


BOOL AIBTA_GroupLaughTime(CAIBrain* pBrain)
{
	_uGroupLaughCounter++;
	if (_uGroupLaughCounter >= _kMAX_GROUP_LAUGHS)
	{
		_fLastDialogPlay = 0.0f;
	}

	f32 fTimeSince = aiutils_FTotalLoopSecs() - _fLastDialogPlay;
	return (fTimeSince >1.2f && fTimeSince <4.0f);
}

BOOL AIBTA_LoadBTAGroup(cchar *pszBTAName) {
	FMemFrame_t MemFrame;
	FResFrame_t ResFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	BTAGroup *pBTAGroup;
	cchar *pszStr;

	MemFrame = fmem_GetFrame();
	ResFrame = fres_GetFrame();

	hFile = fgamedata_LoadFileToFMem( pszBTAName );

	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "AIBTA_LoadBTAGroup(): Failed to load BTA group '%s'.\n", pszBTAName );
		goto _ExitWithError;
	}

	// Get the mesh name
	hTable = fgamedata_GetFirstTableHandle( hFile, "mesh" );

	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "AIBTA_LoadBTAGroup(): Failed to find 'mesh' handle, ignoring table.\n" );
		goto _ExitWithError;
	}

	pszStr = (cchar *) fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if( pszStr == NULL || nDataType != FGAMEDATA_VAR_TYPE_STRING ) {
		DEVPRINTF( "AIBTA_LoadBTAGroup(): Mesh name is not a string, ignoring table.\n" );
		goto _ExitWithError;
	}											    

	// Allocate the BTAGroup
	pBTAGroup = (BTAGroup *) fres_Alloc( sizeof( BTAGroup ) );

	if( pBTAGroup == NULL ) {
		DEVPRINTF( "AIBTA_LoadBTAGroup(): Not enough memory for BTAGroup.\n" );
		goto _ExitWithError;
	}

	pBTAGroup->m_pszMeshName = gstring_Main.AddString( pszStr );

	// Get the sound banks that we want
	if( !_ProcessSoundTables( hFile ) ) {
		// The sound bank load failed for some reason, but don't do anything since it is allowed to fail
	}

	if( !_ProcessBotTalks( hFile, pBTAGroup ) ) {
		goto _ExitWithError;
	}

	flinklist_AddTail( &_BTALinkRoot, pBTAGroup );

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:

	fres_ReleaseFrame( ResFrame );

	return FALSE;
}