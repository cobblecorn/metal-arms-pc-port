//////////////////////////////////////////////////////////////////////////////////////
// WavToADPCM_GC.cpp - 
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
// 06/09/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "WavToADPCM_GC.h"
#include "ErrorLog.h"
#include "dspadpcm.h"

#define _ERROR_HEADING				"WAV TO GC ADPCM FILE COMPILER "

typedef u32 (*lpFunc1)(u32);
typedef u32 (*lpFunc2)(void);
typedef void (*lpFunc3)(s16*, u8*, ADPCMINFO*, u32);
typedef void (*lpFunc4)(u8*, s16*, ADPCMINFO*, u32);
typedef void (*lpFunc5)(u8*, ADPCMINFO*, u32);

static HINSTANCE hDll = NULL;

lpFunc1 getBytesForAdpcmBuffer;
lpFunc1 getBytesForAdpcmSamples;
lpFunc1 getBytesForPcmBuffer;
lpFunc1 getBytesForPcmSamples;
lpFunc1 getSampleForAdpcmNibble;
lpFunc1 getNibbleAddress;
lpFunc2 getBytesForAdpcmInfo;
lpFunc3 encode;
lpFunc4 decode;
lpFunc5 getLoopContext;


CWavToADPCM_GC::CWavToADPCM_GC() {
	m_pData = NULL;
	m_nDataSize = 0;
	m_pHeader = NULL;
	m_nBytesAllocated = 0;
}

CWavToADPCM_GC::~CWavToADPCM_GC() {
	FreeData();
}

BOOL CWavToADPCM_GC::ConvertWavFile( CWaveFile &rWavFile, const CString &rsWavName ) {
	CString sError;
	u32 nSizeRead, nNumSamples, nNumCompressedBytes, i, j, nBytesA, nBytesB;
	u8 *pCompressedSamples, *pSrcData, *pCompressA, *pCompressB;
	s16 *pSrcA, *pDstA, *pSrcB, *pDstB;
	ADPCMINFO AdpcmInfo;

	FreeData();

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// check the passed in wav file
	if( !rWavFile.m_dwSize ||
		rWavFile.m_pwfx->wFormatTag != WAVE_FORMAT_PCM ||
		rWavFile.m_pwfx->wBitsPerSample != 16 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "The wav is an invalid format and can't be converted" );
		return FALSE;
	}

	// load the library
	if( LoadDll() ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "Can't load the GC adpcm dll - you need to install dsptool.dll" );
		return FALSE;
	}
	
	if( rWavFile.m_pwfx->nChannels == 1 ) {
		// this is a mono wav file

		// compute the total number of samples
		nNumSamples = rWavFile.m_dwSize >> 1;
		nNumCompressedBytes = getBytesForAdpcmBuffer( nNumSamples );

		// allocate memory
		m_nBytesAllocated = sizeof( FGCData_WvsFile_Header_t ) +
							nNumCompressedBytes +
							rWavFile.m_dwSize;
		m_pData = (u8 *)fang_MallocAndZero( m_nBytesAllocated, 4 );
		if( !m_pData ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
			rErrorLog.WriteErrorLine( "Could not allocate memory to hold our compiled ADPCM data" );	

			FreeData();
			return FALSE;
		}

		// fixup ptrs
		m_pHeader = (FGCData_WvsFile_Header_t *)m_pData;
		pCompressedSamples = (u8 *)&m_pHeader[1];
		pSrcData = &pCompressedSamples[nNumCompressedBytes];
		
		// read the wav data into our src buffer
		if( !rWavFile.Read( (unsigned char *)pSrcData, rWavFile.m_dwSize, (unsigned long *)&nSizeRead ) ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
			rErrorLog.WriteErrorLine( "Could not read the wav file into our src buffer" );

			FreeData();
			return FALSE;
		}
		FASSERT( nSizeRead == rWavFile.m_dwSize );
		pSrcA = (s16 *)pSrcData;
#if 0
		// endianize our sample data
		for( i=0; i < nNumSamples; i++ ) {
			pSrcA[i] = fang_ConvertEndian( pSrcA[i] );
		}
#endif

		// encode our src data into our compressed buffer
		encode( pSrcA, pCompressedSamples, &AdpcmInfo, nNumSamples );

		// find out how many bytes were actually used
		nNumCompressedBytes = getBytesForAdpcmSamples( nNumSamples );

		// fill in the header
		m_pHeader->fFreqHz = (f32)rWavFile.m_pwfx->nSamplesPerSec;
		m_pHeader->fSecs = rWavFile.m_fSecsOfData;
		m_pHeader->nNumChannels = 1;
		m_pHeader->nNumBytesPerChannel = nNumCompressedBytes;
		m_pHeader->nChunkBytes = (nNumCompressedBytes >= FGCDATA_WVS_MAX_CHUNK_BYTES) ? FGCDATA_WVS_MAX_CHUNK_BYTES : nNumCompressedBytes;
		m_pHeader->nNumChunksPerChannel = (nNumCompressedBytes / m_pHeader->nChunkBytes);
		if( (m_pHeader->nNumChunksPerChannel * m_pHeader->nChunkBytes) != m_pHeader->nNumBytesPerChannel ) {
			// add 1 more chunk to cover the leftover bytes
			m_pHeader->nNumChunksPerChannel++;
		}
		
		// fill in the channel info
		fang_MemCopy( m_pHeader->aChannelInfo[0].anCoef, AdpcmInfo.coef, (sizeof( u16 ) * 16) );
		
		// compute the total data size
		m_nDataSize = sizeof( FGCData_WvsFile_Header_t ) +
					  nNumCompressedBytes;

		// DONE COMPILING THE MONO WAV
	} else {
		// this is a stereo wav
		FASSERT( rWavFile.m_pwfx->nChannels == 2 );

		// compute the total number of samples
		nNumSamples = rWavFile.m_dwSize >> 2;
		nNumCompressedBytes = getBytesForAdpcmBuffer( nNumSamples );

		// allocate memory
		m_nBytesAllocated = sizeof( FGCData_WvsFile_Header_t ) +
							rWavFile.m_dwSize +
							(rWavFile.m_dwSize >> 1) + 
							(nNumCompressedBytes * 2);								
		m_pData = (u8 *)fang_MallocAndZero( m_nBytesAllocated, 4 );
		if( !m_pData ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
			rErrorLog.WriteErrorLine( "Could not allocate memory to hold our compiled ADPCM data" );	

			FreeData();
			return FALSE;
		}

		// fixup ptrs
		m_pHeader = (FGCData_WvsFile_Header_t *)m_pData;
		pSrcData = (u8 *)&m_pHeader[1];
		pCompressedSamples = &pSrcData[rWavFile.m_dwSize];
		pCompressedSamples += (rWavFile.m_dwSize >> 1);
		
		// read the wav data into our src buffer
		if( !rWavFile.Read( (unsigned char *)pSrcData, rWavFile.m_dwSize, (unsigned long *)&nSizeRead ) ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
			rErrorLog.WriteErrorLine( "Could not read the wav file into our src buffer" );

			FreeData();
			return FALSE;
		}
		FASSERT( nSizeRead == rWavFile.m_dwSize );

		// uninterleave the src wav data (and endianize while we are at it)
		pSrcA = (s16 *)pSrcData;
		pSrcB = (s16 *)(pSrcData + 2);
		pDstA = (s16 *)(pSrcData + rWavFile.m_dwSize);
		pDstB = (s16 *)pSrcData;
		for( i=0, j=0; i < nNumSamples; i++, j+=2 ) {
			//pDstA[i] = fang_ConvertEndian( pSrcA[j] );
			//pDstB[i] = fang_ConvertEndian( pSrcB[j] );
			pDstA[i] = pSrcA[j];
			pDstB[i] = pSrcB[j];
		}

		pCompressA = pCompressedSamples;
		pCompressB = &pCompressA[nNumCompressedBytes];

		// encode our A channel into our compressed buffer
		encode( pDstA, pCompressA, &AdpcmInfo, nNumSamples );
		nBytesA = getBytesForAdpcmSamples( nNumSamples );
		// fill in the channel info
		fang_MemCopy( m_pHeader->aChannelInfo[0].anCoef, AdpcmInfo.coef, (sizeof( u16 ) * 16) );

		// allow windows a slice of time (encoding is VERY slow)
		Sleep(0);

		// encode our B channel into our compressed buffer
		encode( pDstB, pCompressB, &AdpcmInfo, nNumSamples );
		nBytesB = getBytesForAdpcmSamples( nNumSamples );
		// fill in the channel info
		fang_MemCopy( m_pHeader->aChannelInfo[1].anCoef, AdpcmInfo.coef, (sizeof( u16 ) * 16) );

		FASSERT( nBytesA == nBytesB );

		// allow windows a slice of time (encoding is VERY slow)
		Sleep(0);
		
		// fill in the header
		m_pHeader->fFreqHz = (f32)rWavFile.m_pwfx->nSamplesPerSec;
		m_pHeader->fSecs = rWavFile.m_fSecsOfData;
		m_pHeader->nNumChannels = 2;
		m_pHeader->nNumBytesPerChannel = nBytesA;
		m_pHeader->nChunkBytes = (nBytesA >= FGCDATA_WVS_MAX_CHUNK_BYTES) ? FGCDATA_WVS_MAX_CHUNK_BYTES : nBytesA;
		m_pHeader->nNumChunksPerChannel = 0;

		i=0; 
		j=0;
		while( i < nBytesA ) {
			nSizeRead = m_pHeader->nChunkBytes;
			if( (i+nSizeRead) >= nBytesA ) {
				nSizeRead = nBytesA - i;
			}
			
			// copy some of our A channel
			fang_MemCopy( &pSrcData[j], &pCompressA[i], nSizeRead );
			j += nSizeRead;

			// copy some of our B channel
			fang_MemCopy( &pSrcData[j], &pCompressB[i], nSizeRead );
			j += nSizeRead;	

			// advance i
			i += nSizeRead;

			// advance our chunk count
			m_pHeader->nNumChunksPerChannel++;
		}
		FASSERT( j == (2 * m_pHeader->nNumBytesPerChannel) );		
		
		// compute the total data size
		m_nDataSize = sizeof( FGCData_WvsFile_Header_t ) +
					  nBytesA + 
					  nBytesB;

		// DONE COMPILING THE STEREO WAV
	}

	// finally endianize the header
	m_pHeader->ChangeEndian();
	
	FreeDLL();
	
	return TRUE;
}

void CWavToADPCM_GC::FreeData() {
	
	if( m_pData ) {
		fang_Free( m_pData );
		m_pData = NULL;
	}
	m_nDataSize = 0;
	m_pHeader = NULL;
	m_nBytesAllocated = 0;

	FreeDLL();
}

// static functions

void CWavToADPCM_GC::FreeDLL( void ) {
	if( hDll ) {
		FreeLibrary( hDll );
		hDll = NULL;
	}
}

// Returns 0 if successful
int CWavToADPCM_GC::LoadDll( void ) {

	if( hDll ) {
		// already loaded
		return 0;
	}

	hDll = LoadLibrary( "dsptool.dll" );
	if( hDll ) {

		getBytesForAdpcmBuffer = (lpFunc1)GetProcAddress( hDll, "getBytesForAdpcmBuffer" );
		if( !getBytesForAdpcmBuffer ) {
			return 1;
		}

		getBytesForAdpcmSamples = (lpFunc1)GetProcAddress( hDll, "getBytesForAdpcmSamples" );
		if( !getBytesForAdpcmSamples ){
			return 2;
		}

		getBytesForPcmBuffer = (lpFunc1)GetProcAddress( hDll, "getBytesForPcmBuffer" );
		if( !getBytesForPcmBuffer ) {
			return 3;
		}

		getBytesForPcmSamples = (lpFunc1)GetProcAddress( hDll, "getBytesForPcmSamples" );
		if( !getBytesForPcmSamples ) {
			return 4;
		}

		getNibbleAddress = (lpFunc1)GetProcAddress( hDll, "getNibbleAddress" );
		if( !getNibbleAddress ) {
			return 5;
		}
	
		getSampleForAdpcmNibble = (lpFunc1)GetProcAddress( hDll, "getSampleForAdpcmNibble" );
		if( !getSampleForAdpcmNibble ) {
			return 6;
		}

		getBytesForAdpcmInfo = (lpFunc2)GetProcAddress( hDll, "getBytesForAdpcmInfo" );
		if( !getBytesForAdpcmInfo ) {
			return 7;
		}

		encode = (lpFunc3)GetProcAddress( hDll, "encode" );
		if( !encode ) {
			return 8;
		}

		decode = (lpFunc4)GetProcAddress( hDll, "decode" );
		if( !encode ) {
			return 9;
		}

		getLoopContext = (lpFunc5)GetProcAddress( hDll, "getLoopContext" );
		if( !getLoopContext ) {
			return 10;
		}

		return 0;
	}
	return 11;
}

#if 0
void main (void)
{
	if( getDll ) {
		clean_up();
		exit(1);
	}
	
	//... put some PCM buffer in memory, reverse the endian if you have to
	u8 *adpcm = (u8*)malloc(getBytesForAdpcmBuffer(samplesToEncode));
	if (adpcm)
	{
	ADPCMINFO adpcminfo;
	// ok.. lets encode it!
	encode(source, adpcm, &adpcminfo, samplesToEncode);
	
	// see how many bytes to store the encoded data stream
	u32 nBytesToStore = getBytesForAdpcmSamples(samplesToEncode);
	
	// don’t need the ADPCM buffer anymore
	free(adpcm);

	clean_up();
}
#endif