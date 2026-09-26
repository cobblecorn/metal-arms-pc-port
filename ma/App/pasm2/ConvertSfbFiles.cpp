//////////////////////////////////////////////////////////////////////////////////////
// ConvertSfbFiles.cpp - 
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
// 06/20/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "ConvertSfbFiles.h"
#include "ErrorLog.h"
#include "fdata.h"
#include "fclib.h"
#include "SeqBank.h"
#include "utils.h"

//====================
// private definitions

#define _ERROR_HEADING				".SFB FILE COMPILER "

typedef union {
	FData_SFxBank_PlayCmd_t Play;

	u8 nPad[32];
} _Cmds_t;// only exists so that we can compute the size of the largest cmd

#define _LARGEST_CMD_BYTES		( sizeof( _Cmds_t ) )

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions

CConvertSfbFile::CConvertSfbFile() {
	m_nNumCmds = 0;
	m_nNumSfxs = 0;
	m_nBytesAllocated = 0;
	m_pMemAllocated = NULL;
	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;
	m_sBasedOnWavBank.Empty();
}

CConvertSfbFile::~CConvertSfbFile() {
	FreeData();
}

BOOL CConvertSfbFile::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ ) {
	CSeqBank SeqBank;
	SeqBank_ToolSettings_t *pToolSettings;
	CString sError, sFilepath;
	int nIndex;
	u32 i, j, nCmdIndex;

	FreeData();

	if( !pFileInfo ) {
		return FALSE;
	}

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// gather some info about the file
	sFilepath = pFileInfo->GetFilePath();

	// read in our SeqBank
	if( !SeqBank.Read( pFileInfo->GetFilePath() ) ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "CSeqBank could not read the bank file." );
		return FALSE;
	}

	m_nNumCmds = SeqBank.GetNumCmds();
	m_nNumSfxs = SeqBank.GetNumFxs();
	pToolSettings = SeqBank.GetFileSettings();

	if( !m_nNumCmds || !m_nNumSfxs ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "The loaded bank is empty, only non-empty banks can be compiled." );
		return FALSE;
	}

	// get the bank name from the tool settings
	m_sBasedOnWavBank = pToolSettings->szBankFileUsed;
	nIndex = m_sBasedOnWavBank.ReverseFind( '\\' );
	if( nIndex >= 0 ) {
		m_sBasedOnWavBank.Delete( 0, nIndex + 1 );
	}
	// remove the extension
	nIndex = m_sBasedOnWavBank.ReverseFind( '.' );
	if( nIndex >= 0 ) {
		m_sBasedOnWavBank.Delete( nIndex, m_sBasedOnWavBank.GetLength() - nIndex );
	}
	if( m_sBasedOnWavBank.IsEmpty() ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "The bank name in the loaded bank is empty. Seq banks must reference a wav bank." );
		return FALSE;
	}

	// compute how much memory we will need to allocate
	m_nBytesAllocated = sizeof( FData_SFxBank_Header_t ) +
						( sizeof( FData_SFxBank_Seq_t ) * m_nNumSfxs ) + 
						( sizeof( FData_SFxBank_Cmd_t ) * m_nNumCmds ) + 
						( _LARGEST_CMD_BYTES * m_nNumCmds ) +
						SEQ_BANK_STRING_LEN + 
						( m_nNumCmds * WAV_BANK_MAX_WAV_FILENAME_LEN ) + 
						( m_nNumSfxs * SEQ_BANK_MAX_FX_NAME_LEN );
	m_nBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nBytesAllocated, 2048 );// allocate a little extra, just in case
	m_pMemAllocated = (u8 *)fang_MallocAndZero( m_nBytesAllocated, 16 );
	if( !m_pMemAllocated ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	// setup some working ptrs
	m_pCompiledFile = m_pMemAllocated;
	m_nCompiledBytes = 0;
	FData_SFxBank_Header_t *pHeader = (FData_SFxBank_Header_t *)m_pCompiledFile;
	FData_SFxBank_Seq_t *pSeq = (FData_SFxBank_Seq_t *)&pHeader[1];
	FData_SFxBank_Cmd_t *pCmd = (FData_SFxBank_Cmd_t *)&pSeq[m_nNumSfxs];
	u8 *pWorkingMem = (u8 *)&pCmd[m_nNumCmds];

	// fill in the header
	pHeader->paSeqs = pSeq;
	pHeader->nNumSFx = m_nNumSfxs;
	pHeader->pszBankName = (cchar *)pWorkingMem;
	fclib_strcpy( (char *)pHeader->pszBankName, m_sBasedOnWavBank );
	pWorkingMem += (m_sBasedOnWavBank.GetLength() + 1);

	// fill in the seq
	SeqBank_Fx_t *pFx;
	nCmdIndex = 0;
	FData_SFxBank_PlayCmd_t *pPlay;
	SeqBank_PlayCmd_t *pPlayCmd;

	for( i=0; i < m_nNumSfxs; i++ ) {
		pFx = SeqBank.GetFxByIndex( i );
		if( !pFx || !pFx->nNumCmds ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
			sError.Format( "There are no commands in the %d'th SFx, check your bank file.", i );
			rErrorLog.WriteErrorLine( sError );			
			return FALSE;
		}
		
		pSeq[i].nNumCmds = pFx->nNumCmds;
		pSeq[i].paCmds = &pCmd[nCmdIndex];
		nCmdIndex += pFx->nNumCmds;

		// copy the sfx name
		pSeq[i].pszName = (cchar *)pWorkingMem;
		fclib_strcpy( (char *)pSeq[i].pszName, pFx->szFxName );
		pWorkingMem += (fclib_strlen( pFx->szFxName ) + 1);

		// create the cmd array
		for( j=0; j < pFx->nNumCmds; j++ ) {
			// align our working mem to 4 (structs should always be 4 byte aligned)
			pWorkingMem = (u8 *)( FMATH_BYTE_ALIGN_UP( (u32)pWorkingMem, 4 ) );

			switch( pFx->paCmds[j].nCmdType ) {
			
			case SEQ_BANK_CMD_TYPE_PLAY:
				pPlayCmd = &pFx->paCmds[j].TypeData.Play;
				pPlay = (FData_SFxBank_PlayCmd_t *)pWorkingMem;
				pWorkingMem += sizeof( FData_SFxBank_PlayCmd_t );

				pSeq[i].paCmds[j].pCmdData = pPlay;

				// copy the wav name
				pPlay->pszWavName = (cchar *)pWorkingMem;
				fclib_strcpy( (char *)pPlay->pszWavName, pPlayCmd->szFilename );
				pWorkingMem += (fclib_strlen( pPlay->pszWavName ) + 1);

				// calculate the wav pitch
				if( pPlayCmd->nPitchPercent == 0 ) {
					pPlay->fFrequencyFactor = 1.0f;
				} else if( pPlayCmd->nPitchPercent > 0 ) {
					pPlay->fFrequencyFactor = ( ((f32)(pPlayCmd->nPitchPercent + 100)) * (1.0f/100.0f) );
				} else if( pPlayCmd->nPitchPercent < -99 ) {
					pPlay->fFrequencyFactor = 1.0f;
					pPlay->fFrequencyFactor /= ( ((f32)pPlayCmd->nPitchPercent) * (1.0f/-100.0f) );
				} else {
					pPlay->fFrequencyFactor = 1.0f - ( ((f32)pPlayCmd->nPitchPercent) * (1.0f/-200.0f) );
				}

				// set the wav volume
				pPlay->fUnitVolume = (f32)pPlayCmd->nVolPercent * (1.0f/100.0f);
				FMATH_CLAMP_UNIT_FLOAT( pPlay->fUnitVolume );
				
				pPlay->nLoops = pPlayCmd->nNumLoops;
				break;
			
			default:
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				sError.Format( "There is an unknown cmd type in the %d'th SFx, %d'th Cmd, check your bank file.", i, j );
				rErrorLog.WriteErrorLine( sError );			
				return FALSE;
				break;
			}

		}
	}
	FASSERT( nCmdIndex == m_nNumCmds );
	
	ConvertPtrsToOffsets( pHeader, bConvertToBigEndian );

	m_nCompiledBytes = pWorkingMem - m_pCompiledFile;
	FASSERT( m_nCompiledBytes <= m_nBytesAllocated );

	return TRUE;
}

u32 CConvertSfbFile::GetDataCRC() {

	if( !m_pCompiledFile ) {
		return FALSE;
	}

	u32 nReturnCRC = fmath_Crc32( 0, (u8 *)m_pCompiledFile, m_nCompiledBytes );

	return nReturnCRC;
}

u32 CConvertSfbFile::GetSizeOfConvertedFile() {

	return m_nCompiledBytes;
}

BOOL CConvertSfbFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

	if( !m_nCompiledBytes ) {
		return FALSE;
	}
	BOOL bCloseFile = FALSE;
	if( !pFileStream ) {
		if( !pszFilename ) {
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if( !pFileStream ) {
			return FALSE;
		}
		bCloseFile = TRUE;
	}
	// write out our file
	fwrite( m_pCompiledFile, m_nCompiledBytes, 1, pFileStream );
	// close our file
	if( bCloseFile ) {
		fclose( pFileStream );
	}

	return TRUE;
}

void CConvertSfbFile::FreeData() {

	if( m_pMemAllocated ) {
		fang_Free( m_pMemAllocated );
		m_pMemAllocated = NULL;
	}
	m_nBytesAllocated = 0;
	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;
	m_nNumCmds = 0;
	m_nNumSfxs = 0;
	m_sBasedOnWavBank.Empty();
}


//==================
// private functions

void CConvertSfbFile::ConvertPtrsToOffsets( FData_SFxBank_Header_t *pHeader, BOOL bConvertToBigEndian ) {
	
	u32 i, j, nBaseAddress;
	FData_SFxBank_Seq_t *pSeq;
	FData_SFxBank_PlayCmd_t *pPlay;

	nBaseAddress = (u32)pHeader;

	for( i=0; i < pHeader->nNumSFx; i++ ) {
		pSeq = (FData_SFxBank_Seq_t *)&pHeader->paSeqs[i];

		pSeq->pszName = (cchar *)UTILS_CONVERT_PTR_TO_OFFSET( pSeq->pszName, nBaseAddress );
		for( j=0; j < pSeq->nNumCmds; j++ ) {
			switch( pSeq->paCmds[j].nCmdType ) {
			case FDATA_SFXBANK_CMD_TYPE_PLAY:
				pPlay = (FData_SFxBank_PlayCmd_t *)pSeq->paCmds[j].pCmdData;

				pPlay->pszWavName = (cchar *)UTILS_CONVERT_PTR_TO_OFFSET( pPlay->pszWavName, nBaseAddress );
				if( bConvertToBigEndian ) {
					pPlay->ChangeEndian();
				}
				break;
			default:
				break;
			}
			pSeq->paCmds[j].pCmdData = (void *)UTILS_CONVERT_PTR_TO_OFFSET( pSeq->paCmds[j].pCmdData, nBaseAddress );

			if( bConvertToBigEndian ) {
				pSeq->paCmds[j].ChangeEndian();
			}
		}
		pSeq->paCmds = (FData_SFxBank_Cmd_t *)UTILS_CONVERT_PTR_TO_OFFSET( pSeq->paCmds, nBaseAddress );

		if( bConvertToBigEndian ) {
			pSeq->ChangeEndian();
		}
	}
	pHeader->pszBankName = (cchar *)UTILS_CONVERT_PTR_TO_OFFSET( pHeader->pszBankName, nBaseAddress );
	pHeader->paSeqs = (FData_SFxBank_Seq_t *)UTILS_CONVERT_PTR_TO_OFFSET( pHeader->paSeqs, nBaseAddress );

	if( bConvertToBigEndian ) {
		pHeader->ChangeEndian();
	}
}

