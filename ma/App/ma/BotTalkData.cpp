//////////////////////////////////////////////////////////////////////////////////////
// BotTalkData.cpp - 
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
// 08/20/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BotTalkData.h"
#include "BotTalkAction.h"
#include "botanim.h"
#include "TalkSystem2.h"

#include "fgamedata.h"
#include "fresload.h"
#include "fanim.h"
#include "fclib.h"
#include "fsndfx.h"
#include "ffile.h"
#include "gstring.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions

CBotTalkData::CBotTalkData() {
	Clear();
}


BOOL CBotTalkData::LoadFromFile( cchar *pszFileName ) {
	u32 uCurActionIdx, nIndex;
	CBotTalkAction *pCurBTA;
	char *pszStreamName = NULL;
	cchar *pszWaveName;
	FAudio_WaveHandle_t hWave;
	cchar *pszFXName;
	FSndFx_FxHandle_t hFXHandle;
	CFAudioStream *pStream = NULL;
	FGameDataTableHandle_t hTableGeneral, hTableAnimList, hTableActionList;

	FResHandle_t hResHandle = fres_GetFrame();
	FMemFrame_t hMemFrame = fmem_GetFrame();
	
	FGameDataFileHandle_t hFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBotTalkData::LoadFromFile() : Could not find file '%s'.\n", pszFileName );
		goto _ExitWithError;
	}

	m_pszSourceFileName = gstring_Main.AddString( pszFileName );

	// Load the general info table, aka the "BotTalk" table.
	hTableGeneral = fgamedata_GetFirstTableHandle( hFile, "BotTalk" );
	if( hTableGeneral == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', no valid 'BotTalk' table.\n", pszFileName);
		goto _ExitWithError;
	}

	if( fgamedata_GetNumFields(hTableGeneral) != 2 ) {
		DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', invalid number of fields in 'BotTalk' table.\n", pszFileName);
		goto _ExitWithError;
	}

	FGameData_VarType_e eVarType;
	m_fTotalTime = *((f32 *)(fgamedata_GetPtrToFieldData(hTableGeneral, 0, eVarType)));

	m_uFlags |= (u32)(*((f32 *)(fgamedata_GetPtrToFieldData(hTableGeneral, 1, eVarType))));
	
	// Load the animation list information, aka the "AnimList" table.
	hTableAnimList = fgamedata_GetFirstTableHandle(hFile, "AnimList");
	if( hTableAnimList == FGAMEDATA_INVALID_TABLE_HANDLE ) {  
		DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', no valid 'AnimList' table.\n", pszFileName);
		goto _ExitWithError;
	}

	m_uAnimCnt = (u32)(*((f32 *)(fgamedata_GetPtrToFieldData(hTableAnimList, 0, eVarType))));

	// NKM - If you hit the assert below, the bot talk CSV is trying to use too many animations.
	FASSERT( m_uAnimCnt <= CBotAnimStackDef::ANIM_USER_COUNT );
	if (m_uAnimCnt >=CBotAnimStackDef::ANIM_USER_COUNT)
	{
		m_uAnimCnt = CBotAnimStackDef::ANIM_USER_COUNT;
	}

	if( fgamedata_GetNumFields(hTableAnimList) != (1 + m_uAnimCnt) ) {
		DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', invalid number of fields in 'AnimList' table.\n", pszFileName);
		goto _ExitWithError;
	}
	m_papAnims = (FAnim_t **)fres_AllocAndZero( sizeof(FAnim_t *) * m_uAnimCnt );
	if( m_papAnims == NULL ) {
		DEVPRINTF("CBotTalkData::LoadFromFile() : Out of memory.\n");
		goto _ExitWithError;
	}

	// load each animation file
	u32 uCurAnimIdx;
	for( uCurAnimIdx = 0; uCurAnimIdx < m_uAnimCnt; ++uCurAnimIdx ) {
		cchar *pszAnimName;
		pszAnimName = (cchar *)fgamedata_GetPtrToFieldData( hTableAnimList, uCurAnimIdx + 1, eVarType );
		m_papAnims[uCurAnimIdx] = (FAnim_t *)fresload_Load( FANIM_RESNAME, pszAnimName );
		if( m_papAnims[uCurAnimIdx] == NULL ) {
			DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', Cannot find animation '%s'.\n", pszFileName, pszAnimName);
			goto _ExitWithError;
		}
	}

	// Load the action list information, aka the "ActionList" table.
	hTableActionList = fgamedata_GetFirstTableHandle(hFile, "ActionList");
	if( hTableActionList == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', no valid 'ActionList' table.\n", pszFileName);
		goto _ExitWithError;
	}

	m_uActionCnt = (u32)(*((f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, 0, eVarType))));
	if( fgamedata_GetNumFields(hTableActionList) != (1 + (BOT_TALK_CSV_NUM_ELEMENTS_PER_ACTION * m_uActionCnt) ) ) {
		DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', invalid number of fields in 'ActionList' table.\n", pszFileName);
		goto _ExitWithError;
	}
	if( m_uActionCnt < 1 ) {
		// there are no actions, don't waste memory
		DEVPRINTF( "CBotTalkData::LoadFromFile() : Error in file '%s', no actions in 'ActionList' table.\n", pszFileName );
		goto _ExitWithError;
	}
	m_paActions = fnew CBotTalkAction[m_uActionCnt];
	if( m_paActions == NULL ) {
		DEVPRINTF("CBotTalkData::LoadFromFile() : Out of memory.\n");
		goto _ExitWithError;
	}

	nIndex = 1;
	for( uCurActionIdx = 0; uCurActionIdx < m_uActionCnt; ++uCurActionIdx ) {
		pCurBTA = &m_paActions[uCurActionIdx];
		pCurBTA->m_fStartTime = *(f32 *)(fgamedata_GetPtrToFieldData( hTableActionList, nIndex, eVarType ));
		pCurBTA->m_fLength = *(f32 *)(fgamedata_GetPtrToFieldData( hTableActionList, nIndex + 1, eVarType ));
		pCurBTA->m_uData1 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData( hTableActionList, nIndex + 2, eVarType )));

		// Little bit of fixup.
		switch( pCurBTA->m_uData1 ) {

		case BOT_TALK_CSV_ACTION_TYPE_ANIMATION:
			// It's an animation.
			pCurBTA->m_uData2 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 3, eVarType)));
			pCurBTA->m_uData3 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 4, eVarType)));
			pCurBTA->m_uData4 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 5, eVarType)));

			// Fix up the user anim slot index.
			pCurBTA->m_uData4 = (CBotAnimStackDef::ANIM_USER_COUNT-1) - (pCurBTA->m_uData4 - 1);

			break;
		
		case BOT_TALK_CSV_ACTION_TYPE_SOUND:
			// It's a sound.
			pCurBTA->m_uData2 = (u32)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 3, eVarType));
			pCurBTA->m_uData3 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 4, eVarType)));
			pCurBTA->m_uData4 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 5, eVarType)));

			// Fix up the 
			pszWaveName = (cchar *)pCurBTA->m_uData2;
			hWave = FAUDIO_INVALID_HANDLE;// this bank was yanked. faudio_GetWaveHandle( CTalkSystem2::s_hTalkBank, pszWaveName );
			if(hWave == FAUDIO_INVALID_HANDLE)
			{
				DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', invalid wave name '%s'.\n", pszFileName, pszWaveName);
			}
			pCurBTA->m_uData2 = (u32)hWave;

			break;

		case BOT_TALK_CSV_ACTION_TYPE_STREAM:
			// It's a streamed audio clip.
			pCurBTA->m_uData2 = (u32)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 3, eVarType));
			pCurBTA->m_uData3 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 4, eVarType)));
			pCurBTA->m_uData4 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 5, eVarType)));

			// Fix up the 
			pszStreamName = (char *)(pCurBTA->m_uData2);

#if 0 // FANG_PLATFORM_WIN

			FFileHandle fp;
			if ((fp = ffile_Open( pszStreamName, FFILE_OPEN_RONLY, TRUE)!= FFILE_INVALID_HANDLE))
			{	//file exists!
				ffile_Close(fp);
				pCurBTA->m_uData2 = (u32)(gstring_Main.AddString(pszStreamName));
			}
			else
			{
				pCurBTA->m_uData2 = 0;
			}

#else
			//on, GC or X, must strip the .wav, then create the stream
			if( pszStreamName ) 
			{
				s32 nStrLength = fclib_strlen(pszStreamName); 
				if ( (nStrLength < 4) || (pszStreamName[nStrLength-4] != '.') )
				{
					DEVPRINTF("CBotTalkData::LoadFromFile() stream name error in '%s", pszFileName);
					FASSERT(0);
				}
				else
				{
					pszStreamName[nStrLength-4] = '\0'; // clips the extension
					char *pszStart = pszStreamName;
					char *pszEnd = &pszStreamName[nStrLength-4]; 
					// find the first slash from the end
					while(--pszEnd != pszStart)
					{
						if (*pszEnd == '\\')
						{
							pszStreamName = pszEnd+1;
							break;
						}
					}
				}
			} 
			else 
			{
				DEVPRINTF("CBotTalkData::LoadFromFile() '%s' file specifies null stream!", pszFileName);
				FASSERT(0);
			}
			pCurBTA->m_uData2 = (u32)gstring_Main.AddString( pszStreamName );
#endif				
			break;
	
		case BOT_TALK_CSV_ACTION_TYPE_SFX:
			// It's a sound FX handle.
			pCurBTA->m_uData2 = (u32)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 3, eVarType));
			pCurBTA->m_uData3 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 4, eVarType)));
			pCurBTA->m_uData4 = (u32)(*(f32 *)(fgamedata_GetPtrToFieldData(hTableActionList, nIndex + 5, eVarType)));

			pszFXName = (cchar *)(pCurBTA->m_uData2);
			hFXHandle = fsndfx_GetFxHandle(pszFXName);

			if( hFXHandle == FSNDFX_INVALID_FX_HANDLE ) {
				DEVPRINTF("CBotTalkData::LoadFromFile() : Error in file '%s', Could not load sound effect '%s'.\n", pszFileName, pszFXName);
			}
			pCurBTA->m_uData2 = (u32)(hFXHandle);
			break;

		default:
			FASSERT_NOW;
			break;
		}

		nIndex += BOT_TALK_CSV_NUM_ELEMENTS_PER_ACTION;
	}
	
	m_uFlags |= BOT_TALK_CSV_FLAG_IS_INITTED;

	fmem_ReleaseFrame( hMemFrame );

	return(TRUE);

_ExitWithError:
	fmem_ReleaseFrame( hMemFrame );
	fres_ReleaseFrame( hResHandle );
	return FALSE;
}

//==================
// private functions

void CBotTalkData::Clear() {

	m_uFlags = BOT_TALK_CSV_FLAG_NONE;

	m_pszSourceFileName = NULL;
	m_uActionCnt = 0;
	m_paActions = NULL;

	m_uAnimCnt = 0;
	m_papAnims = NULL;

	m_fTotalTime = 0.0f;
	m_TS2Next = NULL;					   //For management by TalkSystem2
}
