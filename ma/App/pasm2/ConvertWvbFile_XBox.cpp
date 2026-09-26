//////////////////////////////////////////////////////////////////////////////////////
// ConvertWvbFile_XBox.cpp - 
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
// 06/07/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "fclib.h"
#include "ConvertWvbFile_XBox.h"
#include "WavBank.h"
#include "ErrorLog.h"
#include "utils.h"

#define _ERROR_HEADING				".WVB XBOX FILE COMPILER "


CConvertWvbFile_XBox::CConvertWvbFile_XBox() {
	m_nNumWavs = 0;
	m_nLargestWav = 0;
	m_nSmallestWav = 0;
	m_nBytesAllocated = 0;
	m_pMemAllocated = NULL;
	m_paXboxWaves = NULL;
	m_pBankHeader = NULL;
	m_paWaves = NULL;
	m_nCompiledBytes = 0;
	m_nNumPadBytes = 0;
}

CConvertWvbFile_XBox::~CConvertWvbFile_XBox() {
	FreeData();
}

BOOL CConvertWvbFile_XBox::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ ) {
	CString sFilepath, sTemp;

	FreeData();

	if( !pFileInfo ) {
		return FALSE;
	}

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	sFilepath = pFileInfo->GetFilePath();

	CWavBank WavBank;
	if( !WavBank.Read( sFilepath ) ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not read wave bank file" );
		return FALSE;
	}

	m_nNumWavs = WavBank.GetNumEntries();
	if( !m_nNumWavs ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "There are no wavs in the bank file to compile" );
		return FALSE;
	}
	// allocate memory
	m_nBytesAllocated = sizeof( FDataWvbFile_Bank_t );
	m_nBytesAllocated += sizeof( FDataWvbFile_Wave_t ) * m_nNumWavs;
	m_pMemAllocated = (u8 *)fang_MallocAndZero( m_nBytesAllocated, 4 );
	if( !m_pMemAllocated ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate memory to store compiled file" );
		return FALSE;
	}
	m_paXboxWaves = new CWavToADPCM_XBox[m_nNumWavs];
	if( !m_paXboxWaves ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate xbox adpcm objects to store compiled files" );
		return FALSE;
	}

	// set our pointers
	m_pBankHeader = (FDataWvbFile_Bank_t *)m_pMemAllocated;
	m_paWaves = (FDataWvbFile_Wave_t *)&m_pBankHeader[1];
	
	u32 i, nByteOffset = 0;
	CWaveFile WavFile;
	WavBank_Entry_t *pBankEntry;
	m_nLargestWav = 0;
	m_nSmallestWav = 0;
	for( i=0; i < m_nNumWavs; i++ ) {
		// grab the i'th bank entry
		pBankEntry = WavBank.GetEntryByIndex( i );
		if( !pBankEntry ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
			rErrorLog.WriteErrorLine( "Could not read a wav entry in the wav bank" );
			return FALSE;
		}

		// parse the wav file
		if( !WavFile.OpenFromMemory( (unsigned char *)pBankEntry->pData, pBankEntry->nNumBytes, WAVEFILE_READ ) ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
			rErrorLog.WriteErrorLine( "Trouble reading a wav file in the wav bank" );
			return FALSE;
		}

		// do some error checking
		if( WavFile.m_pwfx->nChannels != pBankEntry->nNumChannels ||
			WavFile.m_pwfx->nSamplesPerSec != pBankEntry->nSamplesPerSec ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
			rErrorLog.WriteErrorLine( "The wav file contained in the bank doesn't match the values stored in the bank header???" );
			return FALSE;
		}

		sTemp.Format( "%s - %s", sFilepath, pBankEntry->szFilename );
		if( !m_paXboxWaves[i].ConvertWavFile( WavFile, sTemp ) ) {
			// error already logged
			return FALSE;
		}
#if 0
		DEVPRINTF( "%s.wav: cbSize %d, nAvgBytesPerSec %d, nBlockAlign %d, nChannels %d, nSamplesPerSec %d, wBitsPerSample %d, wFormatTag %d\n",
			pBankEntry->szFilename,
			WavFile.m_pwfx->cbSize,
			WavFile.m_pwfx->nAvgBytesPerSec,
			WavFile.m_pwfx->nBlockAlign,
			WavFile.m_pwfx->nChannels,
			WavFile.m_pwfx->nSamplesPerSec,
			WavFile.m_pwfx->wBitsPerSample,
			WavFile.m_pwfx->wFormatTag );

		DEVPRINTF( "ADPCM: cbSize %d, nAvgBytesPerSec %d, nBlockAlign %d, nChannels %d, nSamplesPerSec %d, wBitsPerSample %d, wFormatTag %d, nSamplesPerBlock %d\n",
			m_paXboxWaves[i].m_XBWavFormat.WaveFormatEx.cbSize,
			m_paXboxWaves[i].m_XBWavFormat.WaveFormatEx.nAvgBytesPerSec,
			m_paXboxWaves[i].m_XBWavFormat.WaveFormatEx.nBlockAlign,
			m_paXboxWaves[i].m_XBWavFormat.WaveFormatEx.nChannels,
			m_paXboxWaves[i].m_XBWavFormat.WaveFormatEx.nSamplesPerSec,
			m_paXboxWaves[i].m_XBWavFormat.WaveFormatEx.wBitsPerSample,
			m_paXboxWaves[i].m_XBWavFormat.WaveFormatEx.wFormatTag,
			m_paXboxWaves[i].m_XBWavFormat.nSamplesPerBlock);
#endif
		// fill in the wave file header
		fclib_strncpy( m_paWaves[i].szName, pBankEntry->szFilename, FDATA_AUDIO_FILENAME_LEN-1 );
		m_paWaves[i].uLength = m_paXboxWaves[i].m_nDataSize;
		m_paWaves[i].uOffset = nByteOffset;
		m_paWaves[i].uIndex = i;
		m_paWaves[i].fLengthInSeconds = pBankEntry->fSecsOfAudio;
		m_paWaves[i].nFlags = 0;
		m_paWaves[i].fFreqHz = (f32)pBankEntry->nSamplesPerSec;
		m_paWaves[i].nNumChannels = (u8)pBankEntry->nNumChannels;

		// update our byte offset
		nByteOffset += m_paXboxWaves[i].m_nDataSize;

		// update our stats
		if( m_paXboxWaves[i].m_nDataSize > m_nLargestWav ) {
			m_nLargestWav = m_paXboxWaves[i].m_nDataSize;
		}
		if( m_nSmallestWav == 0 ||
			m_paXboxWaves[i].m_nDataSize < m_nSmallestWav ) {
			m_nSmallestWav = m_paXboxWaves[i].m_nDataSize;
		}

		// close the wav file
		WavFile.Close();
	}

	// compute the amount of padding to the file (the wav data section must be a multiple of 72)
	m_nNumPadBytes = nByteOffset / 72;
	m_nNumPadBytes *= 72;
	if( m_nNumPadBytes != nByteOffset ) {
		// we need to pad
		m_nNumPadBytes = nByteOffset - m_nNumPadBytes;
		m_nNumPadBytes = 72 - m_nNumPadBytes;
	} else {
		m_nNumPadBytes = 0;
	}

	// fill in the bank header
	fclib_strncpy( m_pBankHeader->szName, pFileInfo->GetFileTitle(), FDATA_AUDIO_FILENAME_LEN-1 );
	m_pBankHeader->uWaves = m_nNumWavs;
	m_pBankHeader->uDataLength = sizeof( FDX8Data_WaveFormatEx_t ) * m_nNumWavs;
	m_pBankHeader->uDataLength += nByteOffset;
	m_pBankHeader->uDataLength += m_nNumPadBytes;
	
	// compute our total compiled bytes
	m_nCompiledBytes = m_nBytesAllocated;
	m_nCompiledBytes += m_pBankHeader->uDataLength;

	return TRUE;
}

u32 CConvertWvbFile_XBox::GetDataCRC() {

	if( !m_nCompiledBytes ) {
		return FALSE;
	}

	u32 i, nReturnCRC = fmath_Crc32( 0, (u8 *)m_pBankHeader, m_nBytesAllocated );

	for( i=0; i < m_nNumWavs; i++ ) {
		nReturnCRC = fmath_Crc32( nReturnCRC, (u8 *)&m_paXboxWaves[i].m_XBWavFormat, sizeof( FDX8Data_WaveFormatEx_t ) );
	}

	for( i=0; i < m_nNumWavs; i++ ) {
		nReturnCRC = fmath_Crc32( nReturnCRC, (u8 *)m_paXboxWaves[i].m_pData, m_paXboxWaves[i].m_nDataSize );
	}

	return nReturnCRC;
}

u32 CConvertWvbFile_XBox::GetSizeOfConvertedFile() {
	return m_nCompiledBytes;
}

BOOL CConvertWvbFile_XBox::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

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
	/////////////////////
	// write out our file
	
	u32 nBytesWritten = 0;
	// first write out the header and wav header array
	fwrite( m_pBankHeader, m_nBytesAllocated, 1, pFileStream );
	nBytesWritten += m_nBytesAllocated;

	// next write out the FDX8Data_WaveFormatEx_t array
	u32 i;
	for( i=0; i < m_nNumWavs; i++ ) {
		fwrite( &m_paXboxWaves[i].m_XBWavFormat, sizeof( FDX8Data_WaveFormatEx_t ), 1, pFileStream );
		nBytesWritten += sizeof( FDX8Data_WaveFormatEx_t );
	}

	// next write out the ADPCM wav data
	for( i=0; i < m_nNumWavs; i++ ) {
		fwrite( m_paXboxWaves[i].m_pData, m_paXboxWaves[i].m_nDataSize, 1, pFileStream );
		nBytesWritten += m_paXboxWaves[i].m_nDataSize;
	}

	// next write out the pad bytes
	if( m_nNumPadBytes ) {
		utils_PadFileSize( pFileStream, m_nNumPadBytes );
		nBytesWritten += m_nNumPadBytes;
	}
	
	// do a sanity check
	FASSERT( nBytesWritten == m_nCompiledBytes );
	
	/////////////////
	// close our file
	if( bCloseFile ) {
		fclose( pFileStream );
	}

	return TRUE;
}

void CConvertWvbFile_XBox::FreeData() {

	m_nNumWavs = 0;
	m_nLargestWav = 0;
	m_nSmallestWav = 0;
	m_nBytesAllocated = 0;
	if( m_pMemAllocated ) {
		fang_Free( m_pMemAllocated );
		m_pMemAllocated = NULL;
	}
	if( m_paXboxWaves ) {
		delete [] m_paXboxWaves;
		m_paXboxWaves = NULL;
	}
	
	m_pBankHeader = NULL;
	m_paWaves = NULL;
	
	m_nCompiledBytes = 0;
	m_nNumPadBytes = 0;
}

	
