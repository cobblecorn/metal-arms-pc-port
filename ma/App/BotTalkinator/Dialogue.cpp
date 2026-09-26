//////////////////////////////////////////////////////////////////////////////////////
// Dialogue.cpp - 
//
// Author: Michael Starich   
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
// 09/27/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "Dialogue.h"
#include "fclib.h"
#include "ffile.h"
#include "BoneGroup.h"
#include "fsndfx.h"
#include "io.h"
#include "settings.h"
#include "BotTalkCsvFlags.h"

#define FILE_CANWRITE	02
#define YEPITWORKED		0
#define FILE_EXISTS		00

CSoundList *CDialogueInstN::m_apoSoundList[Dialogue_nNumSoundLists];

static const f32 _fBlendTime = 0.25f;
static const f32 _fOOBlendTime = 1.0f / _fBlendTime;

#if 1
// Make and Keep up to eight backup copies
#define MAX_LEN_FILENAME 512
#define NUM_COPIES_TO_KEEP 8
#include <io.h>

// A donation to this tool from pat!
s32 maimain_MakeBackupCopy(const char* pszSrcFileFullPath, const char* pszBackupDir)
{
	char szSrcFileFullPath[MAX_LEN_FILENAME];
	char szBackupFilePathSrc[MAX_LEN_FILENAME];
	char szBackupFilePathDst[MAX_LEN_FILENAME];
	
	FASSERT(pszSrcFileFullPath && strlen(pszSrcFileFullPath) < MAX_LEN_FILENAME - strlen("BACK"));
	
	strcpy(szSrcFileFullPath, pszSrcFileFullPath);
	char *pEx = strrchr(szSrcFileFullPath, '.');
	if (pEx)
	{
		char *pszSrcFile = strrchr(szSrcFileFullPath, '\\');
		if (pszSrcFile && ++pszSrcFile)
		{
			*pEx = '\0';
			pEx++;
			for (int i = (NUM_COPIES_TO_KEEP - 1); i > 0; i--)
			{
				sprintf(szBackupFilePathSrc, "%s\\%s.BACK%02d.%s", pszBackupDir, pszSrcFile, i, pEx);
				
				sprintf(szBackupFilePathDst, "%s\\%s.BACK%02d.%s", pszBackupDir, pszSrcFile, i + 1, pEx);
				
				if (_access(szBackupFilePathSrc, FILE_EXISTS) == YEPITWORKED)  // 0 param means to check for existence only, 0 results mean yes!
				{
					CopyFile(szBackupFilePathSrc, szBackupFilePathDst, FALSE);
				}
			}
			// now backup source
			sprintf(szBackupFilePathDst, "%s\\%s.BACK%02d.%s", pszBackupDir, pszSrcFile, 1, pEx);
			
			if (_access(pszSrcFileFullPath, FILE_EXISTS) == YEPITWORKED)   // is there anything to backup?
			{

				return CopyFile(pszSrcFileFullPath, szBackupFilePathDst, FALSE) !=0;
			}
		}
	}
	return 0;
}
#endif

class CSortableAction 
{
public:
	CSortableAction();

	CString m_sCsvData;
	f32 m_fStartTime;

	static void SortList( CSortableAction *pStart, u32 nCount );
};

CSortableAction::CSortableAction() {
	m_sCsvData.Empty();
	m_fStartTime = 0.0f;
}

void CSortableAction::SortList( CSortableAction *pStart, u32 nCount ) {
	u32 i, j;
	f32 fStartTime;
	CString sTemp;

	if( nCount < 2 ) {
		// nothing to sort
		return;
	}
	// do a buble sort
	for( i=0; i < (nCount-1); i++ ) {
		for( j=(i+1); j < nCount; j++ ) {
			if( pStart[i].m_fStartTime > pStart[j].m_fStartTime ) {
				// swap elements
				fStartTime = pStart[i].m_fStartTime;
				sTemp = pStart[i].m_sCsvData;

				// copy j into i
				pStart[i].m_fStartTime = pStart[j].m_fStartTime;
				pStart[i].m_sCsvData = pStart[j].m_sCsvData;

				// copy the old i data into j
				pStart[j].m_fStartTime = fStartTime;
				pStart[j].m_sCsvData = sTemp;
			}
		}
	}
}

//////////////////////////
// CDialogueInstN METHODS
//////////////////////////

CDialogueInstN::CDialogueInstN() {
	Clear();
	
	// Set up our critical section object.
	InitializeCriticalSection( &m_CriticalSection );	
}

CDialogueInstN::~CDialogueInstN() {

	DeleteCriticalSection( &m_CriticalSection );
}

void CDialogueInstN::Clear() {
	m_strName = "Invalid";
	m_strTitle = "Invalid";
	m_strRace = "Invalid";
	m_strDialogue = "Invalid";
	m_bUseWAV = FALSE;
	m_bIs2d = FALSE;
	m_fTotalTime = 0.0f;
	m_fWavSecs = 0.0f;
	m_fMinTotalTime = 0.0f;
	m_uMilVoice = 0;
	m_fWavStartOffset = 0.0f;
	m_strExportFile.Empty();
}

void CDialogueInstN::Copy( const CDialogueInstN *pCopy ) {

	m_bUseTalkLoop = pCopy->m_bUseTalkLoop;

	m_strName = pCopy->m_strName;
	m_strTitle = pCopy->m_strTitle;
	m_strRace = pCopy->m_strRace;
	m_strDialogue = pCopy->m_strDialogue;

	m_strExportFile = pCopy->m_strExportFile;
	
	m_bUseWAV = pCopy->m_bUseWAV;
	m_bIs2d = pCopy->m_bIs2d;
	m_strWAVFile = pCopy->m_strWAVFile;
	m_uMilVoice = pCopy->m_uMilVoice;
	m_fTotalTime = pCopy->m_fTotalTime;
	m_fWavSecs = pCopy->m_fWavSecs;
	m_fMinTotalTime = pCopy->m_fMinTotalTime;
	m_fWavStartOffset = pCopy->m_fWavStartOffset;
	
	u32 uIdx;
	for( uIdx = 0; uIdx < pCopy->m_uNumGestures; ++uIdx ) {
		m_aoGestureInst[uIdx].Copy( &pCopy->m_aoGestureInst[uIdx] );
	}
	m_uNumGestures = pCopy->m_uNumGestures;
	
	for( uIdx = 0; uIdx < pCopy->m_uNumSounds; ++uIdx ) {
		m_aoSoundInst[uIdx].Copy( &pCopy->m_aoSoundInst[uIdx] );
	}
	m_uNumSounds = pCopy->m_uNumSounds;

	m_pDE = pCopy->m_pDE;
}

BOOL CDialogueInstN::IsDifferent( const CDialogueInstN *pOtherDI ) {

	if( (pOtherDI->m_bUseTalkLoop != m_bUseTalkLoop) ||
		(pOtherDI->m_bUseWAV != m_bUseWAV) ||
		(pOtherDI->m_bIs2d != m_bIs2d) ||
		(pOtherDI->m_uMilVoice != m_uMilVoice) ||
		(pOtherDI->m_fTotalTime != m_fTotalTime) ||
		(pOtherDI->m_fWavSecs != m_fWavSecs) ||
		(pOtherDI->m_fMinTotalTime != m_fMinTotalTime) ||
		(pOtherDI->m_fWavStartOffset != m_fWavStartOffset) ||
		(pOtherDI->m_uNumGestures != m_uNumGestures) ||
		(pOtherDI->m_uNumSounds != m_uNumSounds) ||
		(pOtherDI->m_strName.CompareNoCase( m_strName ) != 0) ||
		(pOtherDI->m_strTitle.CompareNoCase( m_strTitle ) != 0) ||
		(pOtherDI->m_strDialogue.CompareNoCase( m_strDialogue ) != 0) ||
		(pOtherDI->m_strRace.CompareNoCase( m_strRace ) != 0) ||
		(pOtherDI->m_strExportFile.CompareNoCase( m_strExportFile ) != 0) ||
		(pOtherDI->m_strWAVFile.CompareNoCase( m_strWAVFile ) != 0) ) {
		return TRUE;
	}
	for( u32 uIdx = 0; uIdx < m_uNumGestures; ++uIdx ) {
		if( pOtherDI->m_aoGestureInst[uIdx].m_fStartTime != m_aoGestureInst[uIdx].m_fStartTime ) {
			return TRUE;
		}
		if( pOtherDI->m_aoGestureInst[uIdx].m_pGesture != m_aoGestureInst[uIdx].m_pGesture ) {
			return TRUE;
		}
	}
	
	return FALSE;
}

void CDialogueInstN::CalculateTime() {
	char strDialogue[1024];
	char *pszToken;
	CSound *pTempSound;
	CString strToken;
	CString strError;
	
	m_uNumSounds = 0;
	m_fTotalTime = 0.0f;
	
	m_strWAVFile.MakeLower();
	
	if( m_bUseWAV ) {

		if( m_strWAVFile.Right(4).CompareNoCase( ".wav" ) == 0 ) {
			// This is so cheap, we actually open the WAV File and use that to get the length
			FFileHandle hFile;
			hFile = ffile_Open(m_strWAVFile, FFILE_OPEN_RONLY, TRUE);
			if( !FFILE_IS_VALID_HANDLE(hFile) ) {
				DEVPRINTF( "Error: Could not find wave file '%s'.\n", m_strWAVFile );
				m_bUseWAV = FALSE;
			} else {
				WAVEFORMATEX oWaveFormat;
				s32 nSeekOfs = 20;
				ffile_Seek(hFile, nSeekOfs, FFILE_SEEK_SET);
				ffile_Read(hFile, sizeof(WAVEFORMATEX), &oWaveFormat);
				
				m_fWavSecs = ((f32)(ffile_GetFileSize(hFile) - nSeekOfs - sizeof(WAVEFORMATEX) + 4) / ((f32)(oWaveFormat.nSamplesPerSec))) * (8.0f / (f32)(oWaveFormat.wBitsPerSample));
				ffile_Close(hFile);				
			}
		} else {
			FSndFx_FxHandle_t hFXHandle = fsndfx_GetFxHandle( m_strWAVFile );
			if( hFXHandle == FSNDFX_INVALID_FX_HANDLE ) {
				if( m_strWAVFile.IsEmpty() ) {
					// allow this condition, it will be an animation only BTR
					m_fWavSecs = 0.0f;					
				} else {
					// error, switch to the bot talk system
					DEVPRINTF( "Error: Could not find sound effect '%s'.\n", m_strWAVFile );
					m_bUseWAV = FALSE;
				}
			} else {
				FAudio_WaveHandle_t hWave = fsndfx_GetWavHandle(hFXHandle);
				FASSERT(hWave != NULL);
				FAudio_WaveInfo_t oWaveInfo;
				faudio_GetWaveAttributes(hWave, &oWaveInfo);
				m_fWavSecs = oWaveInfo.fLengthInSeconds;				
			}
		}

		if( m_bUseWAV ) {
			// since we are still using the wav system, calculate the total time
			m_fTotalTime = m_fWavSecs;
			if( m_fMinTotalTime > m_fTotalTime ) {
				// allow wav based BTRs to be longer than the wav play time
				m_fTotalTime = m_fMinTotalTime;
			}
		}
	}
	
	if( !m_bUseWAV ) {
		
		if( m_strWAVFile.CompareNoCase( "Invalid" ) == 0 ) {
			m_fTotalTime = 10.0f;
			return;
		}
		fclib_strcpy(strDialogue, m_strDialogue);
		pszToken = strtok(strDialogue, " \n\t");
		
		while( pszToken != NULL ) {
			strToken = pszToken;
			strToken.TrimLeft("-.,!?:<>\"'[]{}()_#$%^&*`~\\/;");
			strToken.TrimRight("-.,!?:<>\"'[]{}()_#$%^&*`~\\/;");
			strToken.MakeLower();
			if( !strToken.IsEmpty() ) {
				if( m_apoSoundList[m_uMilVoice]->FindEntry(strToken, &pTempSound) ) {
					m_aoSoundInst[m_uNumSounds].m_pSound = pTempSound;
					m_aoSoundInst[m_uNumSounds].m_fStartTime = m_fTotalTime;
					m_fTotalTime += pTempSound->m_fLength;
					++m_uNumSounds;
					FASSERT( m_uNumSounds <= _DIALOGUEINST_MAXSOUNDS );
				}
			}
			pszToken = strtok(NULL, " \n\t");
		}
	}
	
	if( m_fTotalTime == 0.0f ) {
		// don't allow zero length BTRs, stick a default time
		m_fTotalTime = 10.0f;
	}
}

BOOL CDialogueInstN::FindGestureAtTime( f32 fTime, cchar *pGestName, CGestureInst **ppoGestureInst ) {
	s32 nCurGestIdx;
	
	for( nCurGestIdx = 0; nCurGestIdx < m_uNumGestures; ++nCurGestIdx ) {
		
		if( m_aoGestureInst[nCurGestIdx].m_pGesture->m_strName.CompareNoCase(pGestName) == 0 ) {
			// It's the right type of gesture...
			if( m_aoGestureInst[nCurGestIdx].m_fStartTime <= fTime ) {
				// It has started by this point in time...
				if( fTime <= m_aoGestureInst[nCurGestIdx].m_fStartTime + m_aoGestureInst[nCurGestIdx].m_pAnimInst->GetTotalTime() ) {
					// It has also not yet ended...
					// It's the right one!
					*ppoGestureInst = & (m_aoGestureInst[nCurGestIdx]);
					return (TRUE);
				}
			}
		}
	}
	
	return (FALSE);
}

BOOL CDialogueInstN::FindSoundAtTime( f32 fTime, CSoundInst **ppoSoundInst ) {
	s32 nCurSoundIdx;
	f32 fSoundEndPos, fSoundStartPos;
	
	for( nCurSoundIdx = 0; nCurSoundIdx < m_uNumSounds; ++nCurSoundIdx ) {
		fSoundStartPos = fSoundEndPos = m_aoSoundInst[nCurSoundIdx].m_fStartTime;
		fSoundEndPos += m_aoSoundInst[nCurSoundIdx].m_pSound->m_fLength;
		if( (fTime >= fSoundStartPos) && (fTime < fSoundEndPos) ) {
			*ppoSoundInst = & (m_aoSoundInst[nCurSoundIdx]);
			return (TRUE);
		}
	}
	
	return (FALSE);
}

BOOL CDialogueInstN::AddGesture( CGesture *pNewGesture, f32 fStartTime ) {

	if( m_uNumGestures == _DIALOGUEINST_MAXGESTURES ) {
		return FALSE;
	}
	
	// Let's start from the end, 
	s32 nCurGestIdx;
	for( nCurGestIdx = m_uNumGestures; nCurGestIdx > 0; --nCurGestIdx ) {
		if( m_aoGestureInst[nCurGestIdx - 1].m_fStartTime < fStartTime ) {
			// We've gotten to the first 
			break;
		}
		
		m_aoGestureInst[nCurGestIdx].Copy( &m_aoGestureInst[nCurGestIdx - 1] );
	}
	// nCurGestIdx now contains the index where we should insert the new gesture inst.
	
	if( !m_aoGestureInst[nCurGestIdx].Init(pNewGesture, fStartTime) ) {
		return FALSE;
	}
	
	++m_uNumGestures;
	RecalcUserAnimSlots();
	
	return TRUE;
}

BOOL CDialogueInstN::RemoveGesture( CGestureInst *pGoneGesture ) {

	u32 uIdx;
	for( uIdx = 0; uIdx < m_uNumGestures; ++uIdx ) {
		if( &m_aoGestureInst[uIdx] == pGoneGesture ) {
			break;
		}
	}
	
	if( uIdx == m_uNumGestures ) {
		// For some reason they asked to delete a gesture inst that wasn't there.
		return FALSE;
	}
	
	// Pull all of the entries after the one that we delete forward.
	u32 uSrcIdx;
	for( uSrcIdx = uIdx + 1; uSrcIdx < m_uNumGestures; ++uSrcIdx ) {
		m_aoGestureInst[uSrcIdx - 1].Copy( &m_aoGestureInst[uSrcIdx] );
	}
		
	--m_uNumGestures;
	RecalcUserAnimSlots();
	
	return TRUE;
}

// Call this when an inst is selected, the user moves a gesture, adds a gesture, or removes a gesture.
void CDialogueInstN::RecalcUserAnimSlots() {

	EnterCriticalSection(&m_CriticalSection);
	
	CalculateTime();
	
	// Find out in what slot each gesture instance should be.
		
	BOOL abUserSlot[NUM_SUPPORTED_SLOTS];
	abUserSlot[0] = TRUE;
	abUserSlot[1] = FALSE;
	abUserSlot[2] = FALSE;
	abUserSlot[3] = FALSE;
	
	f32 afEndTime[NUM_SUPPORTED_SLOTS];
	afEndTime[0] = m_fTotalTime;
	afEndTime[1] = -1.0f;
	afEndTime[2] = -1.0f;
	afEndTime[3] = -1.0f;
	
	CStringArray szGestureNames;
	szGestureNames.SetSize( m_uNumGestures );
	u32 nNumNames = 0, i;

	u32 uCurGIIdx;
	for( uCurGIIdx = 0; uCurGIIdx < m_uNumGestures; ++uCurGIIdx ) {
#if 0
		FASSERT( abUserSlot[0] == TRUE );
		
		// Prune out any already ended gestures from their slots.
		u32 uCurAnimSlot;
		for( uCurAnimSlot = 0; uCurAnimSlot < NUM_SUPPORTED_SLOTS; ++uCurAnimSlot ) {
			FASSERT( abUserSlot[0] == TRUE );
			
			if( abUserSlot[uCurAnimSlot] ) {
				if( afEndTime[uCurAnimSlot] < m_aoGestureInst[uCurGIIdx].m_fStartTime ) {
					if( m_aoGestureInst[uCurGIIdx].m_fStartTime < m_fTotalTime ) {
						FASSERT(uCurAnimSlot != 0);
						abUserSlot[uCurAnimSlot] = FALSE;
						afEndTime[uCurAnimSlot] = -1.0f;
					}
				}
			}
		}
		
		// This is the new loop.  We try to find the lowest priority slot that is not already occupied.  If
		//   we are unable to do so, we find the highest priority slot that is available.
		uCurAnimSlot = (NUM_SUPPORTED_SLOTS-1);
		while( !abUserSlot[uCurAnimSlot] ) {
			FASSERT(uCurAnimSlot > 0);
			--uCurAnimSlot;
		}
		
		if( uCurAnimSlot == 3 ) {
			// The highest priority slot is in use.  We need to find the highest priority slot that is available.
			while( !abUserSlot[uCurAnimSlot] ) {
				FASSERT(uCurAnimSlot > 0);
				--uCurAnimSlot;
			}
			// uCurAnimSlot now contains an available animation slot.
		} else {
			// uCurAnimSlot currently contains the high priority slot in use.  We want the one after it.
			++uCurAnimSlot;
		}
		
		CString strTemp;
		strTemp.Format( "%s, %f", m_strTitle, m_fTotalTime );
		FASSERT_MSG( uCurAnimSlot != 0, strTemp );
		
		FASSERT( uCurAnimSlot < NUM_SUPPORTED_SLOTS );
		
		// We've got a slot to stick it in, let's do it.
		abUserSlot[uCurAnimSlot] = TRUE;
		afEndTime[uCurAnimSlot] = m_aoGestureInst[uCurGIIdx].m_fStartTime + m_aoGestureInst[uCurGIIdx].m_pAnimInst->GetTotalTime();
		
		// This is what we really want to do.
		m_aoGestureInst[uCurGIIdx].m_uAnimSlot = uCurAnimSlot;		
#else
		for( i=0; i < nNumNames; i++ ) {
			if( szGestureNames[i].CompareNoCase( m_aoGestureInst[uCurGIIdx].m_pGesture->m_strSrcFile ) == 0 ) {
				break;
			}
		}
		if( i == nNumNames ) {
			szGestureNames[i] = m_aoGestureInst[uCurGIIdx].m_pGesture->m_strSrcFile;
			nNumNames++;			
		}
		m_aoGestureInst[uCurGIIdx].m_uAnimSlot = (i % NUM_SUPPORTED_SLOTS) + 1;
#endif
	}
		
	LeaveCriticalSection(&m_CriticalSection);
}

BOOL CDialogueInstN::ExportToFile( CString strFileName ) {
	CString sError;
	u32 uDialogueFlags, uActionCount, i, nGestureFlags, nCurAction = 0;
	s32 nIndex;
	char szComputerName[64];
	DWORD nLen = 64;
	SYSTEMTIME SystemTime;
	CStringList astrAnimNames;
	CStringArray astrAnimNames2;
	CGestureInst *pCurGI = NULL;
	CSoundInst *pCurSI = NULL;
	CSortableAction *pActions = NULL;
	FAudio_WaveInfo_t oWaveInfo;
	cchar *pszSoundFileName = NULL;
	POSITION hCurPos;

	if( strFileName == "" ) {
		sError.Format( "Error: ExportToFile.\n" );
		AfxMessageBox(sError);
		return FALSE;
	}
	
	CSettings &oSettings = CSettings::GetCurrent();		// This automagically gets the settings from the file, as well.
	
	CString strFullPath;
	strFullPath = oSettings.GetOutputDir();
	if( strFullPath.GetLength() && strFullPath.Left(1) != "\\" ) {
		strFullPath += "\\";
	}
	strFullPath += strFileName;
	if( strFileName.Left(4) != ".CSV" )
	{
		strFullPath += ".csv";
	}
	strFullPath.MakeUpper();
	
	// see if the file exists
	if( _access( strFullPath, FILE_EXISTS ) == YEPITWORKED ) {
		// file exists, check for file being read only
		if( _access( strFullPath, FILE_CANWRITE ) != YEPITWORKED ) {
			// can't write, must be read only
			sError.Format( "Error: Can't Write File '%s'.\n\n  Existing version is read-only.\n", strFullPath );
			AfxMessageBox( sError );
			return FALSE;
		}
	}
#if 0
	// bust up the output filename to path & file
	CString sPath;
	sPath = strFileName;
	nIndex = sPath.ReverseFind( '\\' );
	if( nIndex >= 0 ) {
		sPath.Delete( nIndex, sPath.GetLength() - nIndex );
	} else {
		sPath = strFileName;		
	}
	maimain_MakeBackupCopy( strFileName, sPath );
#endif

	FILE *pFile = fopen( strFullPath, "w" );
	if( pFile == NULL ) {
		sError.Format( "Error: Could not export file '%s', file could not be opened for writting.\n", strFullPath );
		AfxMessageBox( sError );
		return FALSE;
	}
	
	// Let's look through the gestures and see what flags we should set for the dialogue as a whole.
	uDialogueFlags = m_bUseTalkLoop ? BOT_TALK_CSV_FLAG_HAS_TALK_LOOP : BOT_TALK_CSV_FLAG_NONE;
	for( i = 0; i < m_uNumGestures; i++ ) {
		CGestureInst *pGI = &m_aoGestureInst[i];
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_UPPERBODY ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_LOWERBODY ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_STICKATEND ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_STICK_AT_END;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_NOBLENDOUT ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_NO_BLEND_OUT;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_UPPER_TORSO ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_DRIVES_UPPER_TORSO;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_LOWER_TORSO ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_DRIVES_LOWER_TORSO;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_LEFT_ARM ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_DRIVES_LEFT_ARM;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_RIGHT_ARM ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_DRIVES_RIGHT_ARM;
		}
		if( pGI->m_pGesture->m_uFlags & CGesture::FLAG_HEAD ) {
			uDialogueFlags |= BOT_TALK_CSV_FLAG_DRIVES_HEAD;
		}
	}
	if (this->m_bIs2d)
	{
		uDialogueFlags |= BOT_TALK_CSV_FLAG_2DSOUNDS;
	}

	// format a header for our csv file
	szComputerName[0] = 0;
	GetComputerName( szComputerName, &nLen );
	
	GetLocalTime( &SystemTime );

	fprintf( pFile, "# BotTalkinator Generated File - Do Not Edit By Hand\n" );
	fprintf( pFile, "# Generated by %s on %d/%d/%d %d:%d:%d\n", szComputerName,
																SystemTime.wMonth,
																SystemTime.wDay,
																SystemTime.wYear,
																SystemTime.wHour,
																SystemTime.wMinute,
																SystemTime.wSecond );

	// output the BotTalk table
	fprintf( pFile, "BotTalk,%f,%d,# Total Time, Animation Flags\n\n", m_fTotalTime, uDialogueFlags );
		
	// gather up the animation names into a list
	astrAnimNames.RemoveAll();
	for( i = 0; i < m_uNumGestures; i++ ) {
		POSITION hPos = astrAnimNames.Find( m_aoGestureInst[i].m_pGesture->m_strSrcFile );
		if( hPos == NULL ) {
			astrAnimNames.AddTail( m_aoGestureInst[i].m_pGesture->m_strSrcFile );
			astrAnimNames2.Add( m_aoGestureInst[i].m_pGesture->m_strSrcFile );
		}
	}
	
	// output the AnimList table
	fprintf(pFile, "AnimList,%d,# The number of animation files to follow\n", astrAnimNames.GetCount() );
	hCurPos = astrAnimNames.GetHeadPosition();

	// Print out all of the unique gesture names that we found.
	i = 0;
	while( hCurPos != NULL ) {
		fprintf( pFile, ",%s,# animation file [%d]\n", astrAnimNames.GetAt( hCurPos ), i );
		i++;
		astrAnimNames.GetNext( hCurPos );
	}
	fprintf( pFile, "\n" );

	// Go through our gesture and sound list and output everything that we find.
	uActionCount = m_uNumGestures + m_uNumSounds;
	if( m_bUseWAV && !m_strWAVFile.IsEmpty() ) {
		++uActionCount;		
	}
	
	fprintf( pFile, "ActionList,%d,# The number of actions to follow\n", uActionCount );

	// we have to sort the action list by start time, so allocate a temp array to hold the data
	pActions = new CSortableAction[uActionCount];
	if( !pActions ) {
		// we couldn't allocate our temp array
		sError.Format( "Error: Could not export file '%s' because we ran out of memory.\n", strFullPath );
		AfxMessageBox( sError );
		fclose( pFile );
		return FALSE;
	}	
	
	// gather the wav actions so that we can sort them with the other actions
	if( m_bUseWAV && !m_strWAVFile.IsEmpty() ) {
		if( m_strWAVFile.Right(4).CompareNoCase( ".wav" ) == 0 ) {
			// export a BOT_TALK_CSV_ACTION_TYPE_STREAM
			pActions[nCurAction].m_fStartTime = m_fWavStartOffset;
			pActions[nCurAction].m_sCsvData.Format( ",%f,%f,%d,%s,0,0,", m_fWavStartOffset,
																		 m_fWavSecs,
																		 BOT_TALK_CSV_ACTION_TYPE_STREAM,
																		 m_strWAVFile );
			pActions[nCurAction].m_sCsvData += "# Start Time, Total Time, Action Type, Wav Name, Unused, Unused\n";
			nCurAction++;
		} else {
			// export a BOT_TALK_CSV_ACTION_TYPE_SFX
			pActions[nCurAction].m_fStartTime = m_fWavStartOffset;
			pActions[nCurAction].m_sCsvData.Format( ",%f,%f,%d,%s,0,0,", m_fWavStartOffset, 
																		 m_fWavSecs,
																		 BOT_TALK_CSV_ACTION_TYPE_SFX,
																		 m_strWAVFile );
			pActions[nCurAction].m_sCsvData += "# Start Time, Total Time, Action Type, Sfx Tag Name, Unused, Unused\n";
			nCurAction++;
		}
	}
	// gather the gesture actions so that we can sort them with the other actions
	for( i=0; i < m_uNumGestures; i++ ) {
		pCurGI = &m_aoGestureInst[i];

		// find the animation index in the "AnimList" table for this gesture
		for( nIndex=0; nIndex < astrAnimNames2.GetSize(); nIndex++ ) {
			if( astrAnimNames2.GetAt( nIndex ).CompareNoCase( pCurGI->m_pGesture->m_strSrcFile ) == 0 ) {
				break;
			}
		}
		FASSERT( nIndex < astrAnimNames2.GetSize() );
			
		// set the animation flags for this gesture
		nGestureFlags = BOT_TALK_CSV_FLAG_NONE;
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_UPPERBODY ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_LOWERBODY ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_STICKATEND ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_STICK_AT_END;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_NOBLENDOUT ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_NO_BLEND_OUT;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_UPPER_TORSO ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_DRIVES_UPPER_TORSO;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_LOWER_TORSO ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_DRIVES_LOWER_TORSO;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_LEFT_ARM ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_DRIVES_LEFT_ARM;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_RIGHT_ARM ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_DRIVES_RIGHT_ARM;
		}
		if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_HEAD ) {
			nGestureFlags |= BOT_TALK_CSV_FLAG_DRIVES_HEAD;
		}			
		// export a BOT_TALK_CSV_ACTION_TYPE_ANIMATION
		pActions[nCurAction].m_fStartTime = pCurGI->m_fStartTime;
		pActions[nCurAction].m_sCsvData.Format( ",%f,%f,%d,%d,%d,%d,", pCurGI->m_fStartTime,
																	   pCurGI->m_pAnimInst->GetTotalTime(),
																	   BOT_TALK_CSV_ACTION_TYPE_ANIMATION,
																	   nIndex,
																	   nGestureFlags,
																	   pCurGI->m_uAnimSlot );
		pActions[nCurAction].m_sCsvData += "# Start Time, Total Time, Action Type, Anim File Index, Anim Flags, Anim Slot\n";
		nCurAction++;
	}
	// gather the sound actions so that we can sort them with the other actions
	for( i=0; i < m_uNumSounds; i++ ) {
		pCurSI = &m_aoSoundInst[i];

		faudio_GetWaveAttributes( pCurSI->m_pSound->m_hSound, &oWaveInfo );
		pszSoundFileName = oWaveInfo.szName;
		// export a BOT_TALK_CSV_ACTION_TYPE_SOUND
		pActions[nCurAction].m_fStartTime = pCurGI->m_fStartTime;
		pActions[nCurAction].m_sCsvData.Format( ",%f,%f,%d,%s,0,0,", pCurSI->m_fStartTime,
																	 pCurSI->m_pSound->m_fLength,
																	 BOT_TALK_CSV_ACTION_TYPE_SOUND,
																	 pszSoundFileName );
		pActions[nCurAction].m_sCsvData += "# Start Time, Total Time, Action Type, Sound File Name, Unused, Unused\n";
		nCurAction++;
	}

	// sort the action list
	CSortableAction::SortList( pActions, uActionCount );

	// print out the action list
	for( i=0; i < uActionCount; i++ ) {
		fprintf( pFile, "%s", pActions[i].m_sCsvData );
	}	

	fclose( pFile );

	// delete our temp memory
	delete [] pActions;
	
	return TRUE;
}

// This updates the anim combiners to be at this position.
void CDialogueInstN::SetCurTimePos( f32 fCurTimePos ) {

	EnterCriticalSection(&m_CriticalSection);
	
	u32 uCurAnimSlot;
	CCharacterInfo *pCharInfo = m_pDE->m_poCharInfo;
	for( uCurAnimSlot = 0; uCurAnimSlot < NUM_SUPPORTED_SLOTS; ++uCurAnimSlot ) {
		pCharInfo->m_oAnimCombiner.SetControlValue( pCharInfo->m_anControlId[uCurAnimSlot], 0.0f );
	}
	
#if 0
	if( m_bUseTalkLoop ) {
		pCharInfo->m_oAnimCombiner.AttachToTap( pCharInfo->m_anTapId[3], pCharInfo->m_pTalkLoopAnim );
		pCharInfo->m_pTalkLoopAnim->UpdateTime( fCurTimePos - 0.0f );
		pCharInfo->m_oAnimCombiner.SetControlValue( pCharInfo->m_anControlId[3], 1.0f );
	}
#endif
		
	CGestureInst *pCurGI;
	f32 fSecsSinceStart, fSecsTilEnd;
	u32 uAnimSlotIdx, uControlId, uTapId, uCurGIIdx;
	
	// Find each of the gestures that is active right now, set the combiner to use it, and update its position.
	for( uCurGIIdx = 0; uCurGIIdx < m_uNumGestures; ++uCurGIIdx ) {
		pCurGI = &m_aoGestureInst[uCurGIIdx];

		// compute the time vars
		fSecsSinceStart = fCurTimePos - pCurGI->m_fStartTime;
		fSecsTilEnd = pCurGI->m_fStartTime + pCurGI->m_pAnimInst->GetTotalTime() - fCurTimePos;

		// if we are inside the "play" time window...
		if( (fSecsSinceStart >= 0.0f) && (fSecsTilEnd >= 0.0f) ) {
			FASSERT( pCurGI->m_uAnimSlot <= NUM_SUPPORTED_SLOTS );
			uAnimSlotIdx = NUM_SUPPORTED_SLOTS - pCurGI->m_uAnimSlot;
						
			uControlId = pCharInfo->m_anControlId[uAnimSlotIdx];
			FASSERT( uControlId < NUM_SUPPORTED_SLOTS );
			
			uTapId = pCharInfo->m_anTapId[uAnimSlotIdx];
			
			pCharInfo->m_oAnimCombiner.AttachToTap( uTapId, pCurGI->m_pAnimInst );
			
			if( (pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_UPPERBODY) &&
				(pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_LOWERBODY) ) {
				// this animation drives both the upper and lower body
				pCharInfo->m_oAnimCombiner.Mask_EnableAllBones( uTapId );

			} else {
				// this animation only drives some bones, start by disabling all bones and then add in the driven ones
				pCharInfo->m_oAnimCombiner.Mask_DisableAllBones( uTapId );

				if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_UPPERBODY ) {
					CBoneGroup *pBG = pCharInfo->m_oBoneGroups.FindGroup( "UpperBody" );
					if( pBG ) {
						pBG->EnableBones( &pCharInfo->m_oAnimCombiner, uTapId );
					} else {
						DEVPRINTF( "Undefined bone group '%s' referenced by gesture '%s'.\n", "UpperBody", pCurGI->m_pGesture->m_strName );						
					}
				}
				if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_LOWERBODY ) {
					CBoneGroup *pBG = pCharInfo->m_oBoneGroups.FindGroup( "LowerBody" );
					if( pBG ) {
						pBG->EnableBones( &pCharInfo->m_oAnimCombiner, uTapId );
					} else {
						DEVPRINTF( "Undefined bone group '%s' referenced by gesture '%s'.\n", "LowerBody", pCurGI->m_pGesture->m_strName );
					}
				}
				if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_UPPER_TORSO ) {
					CBoneGroup *pBG = pCharInfo->m_oBoneGroups.FindGroup( "UpperTorso" );
					if( pBG ) {
						pBG->EnableBones( &pCharInfo->m_oAnimCombiner, uTapId );
					} else {
						DEVPRINTF( "Undefined bone group '%s' referenced by gesture '%s'.\n", "UpperTorso", pCurGI->m_pGesture->m_strName );
					}
				}
				if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_LOWER_TORSO ) {
					CBoneGroup *pBG = pCharInfo->m_oBoneGroups.FindGroup( "LowerTorso" );
					if( pBG ) {
						pBG->EnableBones( &pCharInfo->m_oAnimCombiner, uTapId );
					} else {
						DEVPRINTF( "Undefined bone group '%s' referenced by gesture '%s'.\n", "LowerTorso", pCurGI->m_pGesture->m_strName );
					}
				}
				if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_LEFT_ARM ) {
					CBoneGroup *pBG = pCharInfo->m_oBoneGroups.FindGroup( "LeftArm" );
					if( pBG ) {
						pBG->EnableBones( &pCharInfo->m_oAnimCombiner, uTapId );
					} else {
						DEVPRINTF( "Undefined bone group '%s' referenced by gesture '%s'.\n", "LeftArm", pCurGI->m_pGesture->m_strName );
					}
				}
				if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_RIGHT_ARM ) {
					CBoneGroup *pBG = pCharInfo->m_oBoneGroups.FindGroup( "RightArm" );
					if( pBG ) {
						pBG->EnableBones( &pCharInfo->m_oAnimCombiner, uTapId );
					} else {
						DEVPRINTF( "Undefined bone group '%s' referenced by gesture '%s'.\n", "RightArm", pCurGI->m_pGesture->m_strName );
					}
				}
				if( pCurGI->m_pGesture->m_uFlags & CGesture::FLAG_HEAD ) {
					CBoneGroup *pBG = pCharInfo->m_oBoneGroups.FindGroup( "Head" );
					if( pBG ) {
						pBG->EnableBones( &pCharInfo->m_oAnimCombiner, uTapId );
					} else {
						DEVPRINTF( "Undefined bone group '%s' referenced by gesture '%s'.\n", "Head", pCurGI->m_pGesture->m_strName );
					}
				}
			}
						
			pCurGI->m_pAnimInst->UpdateTime( fSecsSinceStart );
			
			if( fSecsSinceStart < _fBlendTime ) {
				pCharInfo->m_oAnimCombiner.SetControlValue( uControlId, fSecsSinceStart * _fOOBlendTime );
			} else if( fSecsTilEnd < _fBlendTime ) {
				pCharInfo->m_oAnimCombiner.SetControlValue( uControlId, fSecsTilEnd * _fOOBlendTime );
			} else {
				pCharInfo->m_oAnimCombiner.SetControlValue( uControlId, 1.0f );
			}
		}
	}
	
	pCharInfo->m_oAnimCombiner.ComputeMtxPalette();
	
	LeaveCriticalSection( &m_CriticalSection );
}

void CDialogueInstN::Draw() {
	m_pDE->m_poCharInfo->Draw();
}



//////////////////////////
// CDialogueEntryN METHODS
//////////////////////////

CDialogueEntryN::CDialogueEntryN() {
	m_lpDialogues.RemoveAll();
	FASSERT(m_lpDialogues.GetCount() == 0);
}

CDialogueEntryN::~CDialogueEntryN() {

	while( !m_lpDialogues.IsEmpty() ) {
		delete(CDialogueInstN *)(m_lpDialogues.RemoveTail());
	}
}

BOOL CDialogueEntryN::AddEntry( CDialogueInstN *pNewDI ) {
	FASSERT( pNewDI->m_strName.CompareNoCase( m_poCharInfo->m_strCharName ) == 0 );

	CDialogueInstN *pTempDI = new CDialogueInstN;
	if( pTempDI == NULL ) {
		return (FALSE);
	}
	
	pTempDI->Copy( pNewDI );
	pTempDI->CalculateTime();
	m_lpDialogues.AddTail( pTempDI );
	
	return (TRUE);
}

BOOL CDialogueEntryN::DeleteEntry( CDialogueInstN *pDelDI ) {
	FASSERT(pDelDI->m_strName.CompareNoCase( m_poCharInfo->m_strCharName ) == 0);
	
	POSITION hPos;
	hPos = m_lpDialogues.Find(pDelDI);
	
	if( hPos == NULL ) {
		return (FALSE);
	}
	
	m_lpDialogues.RemoveAt(hPos);
	return (TRUE);
}


//////////////////////////
// CDialogueListN METHODS
//////////////////////////

CDialogueEntryN *CDialogueListN::s_pCurDE = NULL;

CDialogueListN::CDialogueListN() {
	m_bIsInitialized = FALSE;
	FASSERT( m_lpDialogueEntries.GetCount() == 0 );
}

CDialogueListN::~CDialogueListN() {
	UnInit();
}

BOOL CDialogueListN::Init( u32 uMaxGestures, u32 uMaxSounds ) {

	m_oCharInfoList.Init( CDialogueListN_uMaxCharacters, uMaxGestures );
	
	u32 uIdx;
	for( uIdx = 0; uIdx < Dialogue_nNumSoundLists; ++uIdx ) {
		if( !m_aoSoundList[uIdx].Init(uMaxSounds) ) {
			return (FALSE);
		}
	}
	
	m_bIsInitialized = TRUE;
	
	return (TRUE);
}

void CDialogueListN::UnInit() {
	u32 uIdx;

	for( uIdx = 0; uIdx < Dialogue_nNumSoundLists; ++uIdx ) {
		m_aoSoundList[uIdx].UnInit();
	}
	
	m_oCharInfoList.Uninit();
	
	while( !m_lpDialogueEntries.IsEmpty() ) {
		delete(CDialogueEntryN *)(m_lpDialogueEntries.RemoveTail());
	}
	m_lpDialogueEntries.RemoveAll();
}

static BOOL ParseString( CString &strToParse, CString &strToken, CString &strDelimiters ) {

	if( strToParse.GetLength() == 0 ) {
		// This is optional whether you think this should return FALSE or not.
		return (FALSE);
	}
	
	s32 nDelimiter = strToParse.FindOneOf(strDelimiters);
	if( nDelimiter == -1 ) {
		strToken = strToParse;
		strToParse = "";
		
		return (TRUE); 
	}
	
	strToken = strToParse.Left(nDelimiter);
	
	s32 nStrLength = strToParse.GetLength();
	strToParse = strToParse.Right(nStrLength - nDelimiter - 1);
	
	return (TRUE);
}

BOOL CDialogueListN::LoadDialoguesFromFile( const char *pszFileName ) {
	FILE *fInFile;
	u32 uCurLineNum;
	
	fInFile = fopen(pszFileName, "r");
	FASSERT(fInFile != NULL);
	
	char sSigLine[1024];
	fscanf(fInFile, "%[^\n]\n", sSigLine);
	if( strcmp(sSigLine, "### BOTTALKINATOR DIALOGUE FILE ###") != 0 ) {
		return (FALSE);
	}
	
	char sLine[1024];
	CString sError;
	CString strLine;
	CString strToken;
	CString strDelimiters("^\n");
	
	CDialogueInstN oDI;

	// parse the file - NOTE: the format is clearly commented in the function - SaveToFile()
	
	uCurLineNum = 0;
	while( !feof(fInFile) ) {
		++uCurLineNum;
		
		if( fscanf(fInFile, "%[^\n]\n", sLine) != 1 ) {
			sError.Format("Error in input file: Line %d, Invalid line.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		}
		
		strLine = sLine;
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Bot Name field.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		} else {
			oDI.m_strName = strToken;
		}
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Bot Race field.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		} else {
			oDI.m_strRace = strToken;
		}
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Title field.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		} else {
			oDI.m_strTitle = strToken;
		}
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Dialogue field.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		} else {
			oDI.m_strDialogue = strToken;
		}
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Export File field.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		}
		
		if( strToken.CompareNoCase( "None" ) == 0 ) {
			oDI.m_strExportFile = "";
		} else {
			oDI.m_strExportFile = strToken;
		}
				
		u32 uNumGestures = 0;
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Gesture count.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		} else {
			uNumGestures = atoi(strToken);
		}
				
		CCharacterInfo *pCI = m_oCharInfoList.FindCharacterInfo(oDI.m_strName);
		if( pCI == NULL ) {
			CString strTemp;
			strTemp.Format("CDialogueListN::LoadDialoguesFromFile() : Error: Undefined name '%s' for dialogue.\n", oDI.m_strName);
			AfxMessageBox(strTemp);
			continue;
		}
		
		u32 uIdx;
		BOOL bError = FALSE;
		CString strGestName;
		f32 fGestTime;
		oDI.m_uNumGestures = 0;
		for( uIdx = 0; uIdx < uNumGestures; ++uIdx ) {

			if( !ParseString(strLine, strToken, strDelimiters) ) {
				sError.Format("Error in input file: Line %d, Gesture count.\n", uCurLineNum);
				AfxMessageBox(sError);
				bError = TRUE;
			} else {
				strGestName = strToken;
			}
			
			if( !ParseString(strLine, strToken, strDelimiters) ) {
				sError.Format("Error in input file: Line %d, Gesture time stamp.\n", uCurLineNum);
				AfxMessageBox(sError);
				bError = TRUE;
			} else {
				fGestTime = (f32)(atof(strToken));
			}
			
			if( bError ) {
				continue;
			}
			
			CGesture *poRetGest;
			if( pCI->FindGesture(strGestName, &poRetGest) ) {
				oDI.AddGesture(poRetGest, fGestTime);
			} else {
				sError.Format("CDialogueListN::LoadDialoguesFromFile() : Error: Undefined name '%s' for dialogue.\n", oDI.m_strName);
				AfxMessageBox(sError);
				continue;
			}
		}
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Use WAV file flag.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		}
		if( strToken == "0" ) {
			oDI.m_bUseWAV = FALSE;
			oDI.m_bIs2d = FALSE;
		} else if( strToken == "1" ) {
			oDI.m_bUseWAV = TRUE;
			oDI.m_bIs2d = FALSE;
		} else if( strToken == "2" ) {
			oDI.m_bUseWAV = FALSE;
			oDI.m_bIs2d = TRUE;
		} else if( strToken == "3" ) {
			oDI.m_bUseWAV = TRUE;
			oDI.m_bIs2d = TRUE;
		} else {
			sError.Format("Error in input file: Line %d, Use WAV file flag.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		}
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Sound data field.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		}
		if( oDI.m_bUseWAV ) {
			oDI.m_strWAVFile = strToken;
			oDI.m_uMilVoice = 0;
		} else {
			oDI.m_strWAVFile = "";
			oDI.m_uMilVoice = atoi(strToken);
			FMATH_CLAMP( oDI.m_uMilVoice, 0, (Dialogue_nNumSoundLists-1) );
		}
		
		oDI.m_uNumSounds = 0;
		
		if( !ParseString(strLine, strToken, strDelimiters) ) {
			sError.Format("Error in input file: Line %d, Use talk loop flag.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		}
		if( strToken == "0" ) {
			oDI.m_bUseTalkLoop = FALSE;
		} else if( strToken == "1" ) {
			oDI.m_bUseTalkLoop = TRUE;
		} else {
			sError.Format("Error in input file: Line %d, Use talk loop flag.\n", uCurLineNum);
			AfxMessageBox(sError);
			continue;
		}

		if( ParseString( strLine, strToken, strDelimiters ) ) {
			oDI.m_fMinTotalTime = (f32)atof( strToken );
		} else {
			oDI.m_fMinTotalTime = 0.0f;
		}

		if( ParseString( strLine, strToken, strDelimiters ) ) {
			oDI.m_fWavStartOffset = (f32)atof( strToken );
		} else {
			oDI.m_fWavStartOffset = 0.0f;
		}
		
		AddDialogueInst( &oDI );
	}
	
	fclose(fInFile);
	
	return (TRUE);
}

BOOL CDialogueListN::LoadCharInfoFromFile( const char *pszFileName ) {
	return m_oCharInfoList.LoadFromFile( pszFileName );
}

BOOL CDialogueListN::LoadSoundsFromFile( const char *pszFileName ) {
	BOOL bRetVal = TRUE;
	u32 uIdx;
	
	// JUSTIN: Add the loading of the bank here.
	m_hBank = faudio_LoadBank("MS11_BoTalk");
	
	for( uIdx = 0; uIdx < Dialogue_nNumSoundLists; ++uIdx) {
		bRetVal &= m_aoSoundList[uIdx].LoadFromFile(pszFileName, m_hBank, uIdx);
	}

	return (bRetVal);
}

BOOL CDialogueListN::AddDialogueInst( CDialogueInstN *pNewDI ) {
	CDialogueEntryN *poDE;

	poDE = FindDialogueEntry( pNewDI->m_strName );
	if( poDE == NULL ) {
		poDE = AddDialogueEntry( pNewDI->m_strName );
	}
	
	pNewDI->m_pDE = poDE;
	if( poDE == NULL ) {
		return FALSE;
	}
	
	return ( poDE->AddEntry(pNewDI) );
}

CDialogueEntryN *CDialogueListN::AddDialogueEntry( CString strName ) {

	CDialogueEntryN *poDE = new CDialogueEntryN();
	// JUSTIN: Do something with the strName.
	if( poDE == NULL ) {
		return (NULL);
	}
	
	poDE->m_poCharInfo = m_oCharInfoList.FindCharacterInfo(strName);
	FASSERT(poDE->m_poCharInfo != NULL);
	
	m_lpDialogueEntries.AddTail(poDE);
	return (poDE);
}

CDialogueEntryN *CDialogueListN::FindDialogueEntry( CString strName ) {
	CDialogueEntryN *poDE;

	POSITION hPos = m_lpDialogueEntries.GetHeadPosition();
	while( hPos != NULL ) {
		poDE = (CDialogueEntryN *)(m_lpDialogueEntries.GetAt(hPos));
		if( poDE->m_poCharInfo->m_strCharName.CompareNoCase( strName ) == 0 ) {
			return (poDE);
		}
		m_lpDialogueEntries.GetNext(hPos);
	}
	return (NULL);
}

BOOL CDialogueListN::SaveToFile( const CString strOutputFileName ) {
	FILE *fOutFile;
	u32 uCurGest;
	CString sError;
	
	// see if the file exists
	if( _access( strOutputFileName, FILE_EXISTS ) == YEPITWORKED ) {
		// file exists, check for file being read only
		if( _access( strOutputFileName, FILE_CANWRITE ) != YEPITWORKED ) {
			// can't write, must be read only
			sError.Format( "Error: Can't Write File '%s'.\n\n  Existing version is read-only.\n", strOutputFileName );
			AfxMessageBox( sError );
			return FALSE;
		}
	}

	// bust up the output filename to path & file
	CString sPath;
	int nIndex;
	sPath = strOutputFileName;
	nIndex = sPath.ReverseFind( '\\' );
	if( nIndex >= 0 ) {
		sPath.Delete( nIndex, sPath.GetLength() - nIndex );
	} else {
		sPath = strOutputFileName;		
	}
	maimain_MakeBackupCopy( strOutputFileName, sPath );

	fOutFile = fopen( strOutputFileName, "w" );
	if( fOutFile == NULL ) {
		sError.Format("Error: Could not export file '%s', file could not be opened for writting.\n", strOutputFileName );
		AfxMessageBox( sError );
		return FALSE;
	}
	
	fprintf( fOutFile, "### BOTTALKINATOR DIALOGUE FILE ###\n" );
	
	// HERE IS THE FORMAT OF THE BTDIALOGUE.TXT FILE:
	// EACH LINE REPRESENTS ALL THE DATA NEEDED FOR AN INDIVIDUAL BOT TALK INST (CDialogueInstN).
	// BOT NAME (STRING)^
	// BOT RACE (STRING)^
	// TITLE (STRING)^
	// DIALOGUE (STRING)^
	// CSV EXPORT FILENAME (STRING)^		- WILL BE SET TO 'NONE' IF BLANK
	// NUMBER OF GESTURES (INT)^
	//		FOR EACH GESTURE:
	//			GESTURE NAME (STRING)^
	//			GESTURE START TIME (FLOAT)^
	// USE WAVE (0 | 1)^
	//		IF 1:
	//			NAME OF WAVE FILE (STRING)^
	//		IF 0:
	//			INDEX OF MIL VOICE (INT)^
	// USE TALK LOOP (0 | 1)^
	// MIN TOTAL TIME (FLOAT)^
	// WAV START OFFSET (FLOAT)
	CDialogueEntryN *poDE;
	CDialogueInstN *poDI;
	POSITION pPosEntry = m_lpDialogueEntries.GetHeadPosition();
	while( pPosEntry != NULL ) {

		poDE = (CDialogueEntryN *)m_lpDialogueEntries.GetNext( pPosEntry );
		POSITION pPosInst = poDE->m_lpDialogues.GetHeadPosition();
		while( pPosInst != NULL ) {
			poDI = (CDialogueInstN *)poDE->m_lpDialogues.GetNext( pPosInst );
			fprintf( fOutFile, "%s^%s^%s^%s", poDI->m_strName, poDI->m_strRace, poDI->m_strTitle, poDI->m_strDialogue );
			
			if( poDI->m_strExportFile == "" ) {
				fprintf( fOutFile, "^None" );
			} else {
				fprintf( fOutFile, "^%s", poDI->m_strExportFile );
			}
			
			fprintf( fOutFile, "^%d", poDI->m_uNumGestures );
			for( uCurGest = 0; uCurGest < poDI->m_uNumGestures; ++uCurGest ) {
				fprintf( fOutFile, "^%s", poDI->m_aoGestureInst[uCurGest].m_pGesture->m_strName );
				fprintf( fOutFile, "^%2.2f", poDI->m_aoGestureInst[uCurGest].m_fStartTime );
			}
			switch (((!!poDI->m_bIs2d) << 1)| (!!poDI->m_bUseWAV))
			{
			case 3:
				fprintf( fOutFile, "^%c", '3');
				break;
			case 2:
				fprintf( fOutFile, "^%c", '2');
				break;
			case 1:
				fprintf( fOutFile, "^%c", '1');
				break;
			case 0:
				fprintf( fOutFile, "^%c", '0');
				break;
			}
			
			if( poDI->m_bUseWAV ) {
				fprintf(fOutFile, "^%s", poDI->m_strWAVFile);
			} else {
				fprintf(fOutFile, "^%d", poDI->m_uMilVoice);
			}
			
			fprintf( fOutFile, "^%c", poDI->m_bUseTalkLoop ? '1' : '0' );

			fprintf( fOutFile, "^%.3f", poDI->m_fMinTotalTime );

			fprintf( fOutFile, "^%.3f", poDI->m_fWavStartOffset );

			// the end of this line
			fprintf( fOutFile, "\n" );
		}
	}
	
	fclose( fOutFile );

	return TRUE;
}



