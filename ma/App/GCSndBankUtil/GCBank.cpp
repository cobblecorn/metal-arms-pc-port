//////////////////////////////////////////////////////////////////////////////////////
// GCBank.cpp - 
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
// 06/14/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "GCBank.h"
#include "fdata.h"
#include "fclib.h"

CGCBank::CGCBank() {
}

CGCBank::~CGCBank() {
}

BOOL CGCBank::CreateGCBank( CWavBank &rWavBank,
						    CParseHeader &rHeader,
							CString &rsBankName,
							CString &rsOutputFilename ) {
	u32 nBytesNeeded, nNumEntries, i;
	u8 *pMemAllocated;
	FDataWvbFile_Bank_t *pBank;
	FDataWvbFile_Wave_t *pWave;
	u32 *pnIDs;
	int nID;
	WavBank_Entry_t *pEntry;
	FILE *pFile;

	nNumEntries = rWavBank.GetNumEntries();

	// Allocate the needed memory
	nBytesNeeded = sizeof( FDataWvbFile_Bank_t ) + 
				   ( nNumEntries * sizeof( FDataWvbFile_Wave_t ) ) + 
				   ( (nNumEntries + 1) * sizeof( u32 ) );
	pMemAllocated = (u8 *)malloc( nBytesNeeded );
	if( !pMemAllocated ) {
		return FALSE;
	}

	// zero out memory
	fang_MemZero( pMemAllocated, nBytesNeeded );

	// Setup our ptrs
	pBank = (FDataWvbFile_Bank_t *)pMemAllocated;
	pWave = (FDataWvbFile_Wave_t *)&pBank[1];
	pnIDs = (u32 *)&pWave[nNumEntries];

	// fill in the header
	fclib_strncpy( pBank->szName, rsBankName, FDATA_AUDIO_FILENAME_LEN-1 );
	pBank->uWaves = nNumEntries;
	pBank->uDataLength = (nNumEntries + 1) * sizeof( u32 );

	// fill in the group id
	nID = rHeader.FindGroupIndex( rsBankName );
	if( nID < 0 ) {
		free( pMemAllocated );
		return FALSE;
	}
	pnIDs[0] = (u32)nID;
	pnIDs[0] = fang_ConvertEndian( pnIDs[0] );

	// walk the waves filling out the wavdata and finding the IDs
	for( i=0; i < nNumEntries; i++ ) {
		// get the entry ptr
		pEntry = rWavBank.GetEntryByIndex( i );
		if( !pEntry ) {
			free( pMemAllocated );
			return FALSE;
		}
		fclib_strncpy( pWave[i].szName, pEntry->szFilename, FDATA_AUDIO_FILENAME_LEN );
		pWave[i].uLength = sizeof( u32 );
		pWave[i].uOffset = (i+1) * sizeof( u32 );
		pWave[i].uIndex = i;
		pWave[i].fLengthInSeconds = pEntry->fSecsOfAudio;
		pWave[i].nFlags = 0;
		pWave[i].fFreqHz = (f32)pEntry->nSamplesPerSec;
		pWave[i].nNumChannels = (u8)pEntry->nNumChannels;
		
		nID = rHeader.FindSfxIndex( pEntry->szFilename );
		if( nID < 0 ) {
			free( pMemAllocated );
			return FALSE;
		}
		pnIDs[i+1] = (u32)nID;

		// endianize the Wave & the id
		pWave[i].ChangeEndian();
		pnIDs[i+1] = fang_ConvertEndian( pnIDs[i+1] );
	}
	
	// endianize the header
	pBank->ChangeEndian();
	
	// write out our file
	pFile = _tfopen( rsOutputFilename, _T("wb") );
	if( !pFile ) {
		free( pMemAllocated );
		return FALSE;
	}
	fwrite( pMemAllocated, nBytesNeeded, 1, pFile );
	fclose( pFile );

	free( pMemAllocated );

	return TRUE;
}
