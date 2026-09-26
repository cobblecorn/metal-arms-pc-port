//////////////////////////////////////////////////////////////////////////////////////
// WavToADPCM_Xbox.cpp - 
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
#include "WavToADPCM_Xbox.h"
#include <mmreg.h>
#include <msacm.h>
#include "ErrorLog.h"

#define _ERROR_HEADING				"WAV TO XB ADPCM FILE COMPILER "

CWavToADPCM_XBox::CWavToADPCM_XBox() {
	m_pData = NULL;
	m_nDataSize = 0;
	fang_MemZero( &m_XBWavFormat, sizeof( FDX8Data_WaveFormatEx_t ) );	
}

CWavToADPCM_XBox::~CWavToADPCM_XBox() {
	FreeData();
}

BOOL CWavToADPCM_XBox::ConvertWavFile( CWaveFile &rWavFile, const CString &rsWavName ) {
	CString sError;

	FreeData();

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// check the passed in wav file
	if( !rWavFile.m_dwSize ||
		rWavFile.m_pwfx->wFormatTag != WAVE_FORMAT_PCM ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "The wav is an invalid format and can't be converted" );
		return FALSE;
	}

	HINSTANCE library = LoadLibrary( "xbadpcm.acm" );
	if( !library ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "Can't load the Xbox acm - you need to install the XBADPCM audio codec" );
		return FALSE;
	}

	ACMDRIVERPROC driver_proc = (ACMDRIVERPROC)GetProcAddress( library, "DriverProc" );
	if( !driver_proc ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "Can't extract driver proc from the XBADPCM audio codec" );

		FreeLibrary( library );
		return FALSE;
	}
	HACMDRIVERID driver_id = NULL;
	MMRESULT r = acmDriverAdd( &driver_id,
							   library,
							   (LPARAM)driver_proc,
							   0, 
							   ACM_DRIVERADDF_FUNCTION | ACM_DRIVERADDF_LOCAL );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmDriverAdd failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );

		FreeLibrary( library );
		return FALSE;
	}

	HACMDRIVER driver = NULL;
	r = acmDriverOpen( &driver, driver_id, 0 );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmDriverOpen failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );

		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		return FALSE;
	}

	WAVEFORMATEX src_fmt;
	src_fmt = *rWavFile.GetFormat();
	
	memset( &m_XBWavFormat, 0, sizeof( FDX8Data_WaveFormatEx_t ) );
	m_XBWavFormat.WaveFormatEx = src_fmt;

	m_XBWavFormat.WaveFormatEx.wBitsPerSample = 4;

	r = acmFormatSuggest( driver,
						  &src_fmt,
						  &m_XBWavFormat.WaveFormatEx,
						  sizeof( FDX8Data_WaveFormatEx_t ),
						  ACM_FORMATSUGGESTF_NSAMPLESPERSEC | ACM_FORMATSUGGESTF_NCHANNELS | ACM_FORMATSUGGESTF_WBITSPERSAMPLE );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmFormatSuggest failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );
		
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		return FALSE;

#if 0
		// try forcing a format
		m_XBWavFormat.nSamplesPerBlock = 64;
		m_XBWavFormat.WaveFormatEx.cbSize = 2;
		m_XBWavFormat.WaveFormatEx.nChannels = src_fmt.nChannels;
		m_XBWavFormat.WaveFormatEx.nBlockAlign = 36 * src_fmt.nChannels;
		m_XBWavFormat.WaveFormatEx.wBitsPerSample = 4;
		m_XBWavFormat.WaveFormatEx.wFormatTag = 0x0069;
		m_XBWavFormat.WaveFormatEx.nSamplesPerSec = src_fmt.nSamplesPerSec;
		m_XBWavFormat.WaveFormatEx.nAvgBytesPerSec = (src_fmt.nSamplesPerSec * m_XBWavFormat.WaveFormatEx.nBlockAlign) >> 6;
#endif
	}

	HACMSTREAM stream;
	r = acmStreamOpen( &stream,
					   driver,
					   &src_fmt,
					   &m_XBWavFormat.WaveFormatEx,
					   NULL,
					   NULL,
					   NULL,
					   ACM_STREAMOPENF_NONREALTIME );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmStreamOpen failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );
		
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		return FALSE;
	}

	m_nDataSize = 0;
	r = acmStreamSize( stream, rWavFile.m_dwSize, (unsigned long *)&m_nDataSize, ACM_STREAMSIZEF_SOURCE );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmStreamSize failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );

		acmStreamClose( stream, 0 );
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		return FALSE;
	}

	m_pData = (u8 *)fang_MallocAndZero( m_nDataSize, 4 );
	if( !m_pData ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "Could not allocate memory to hold our compiled ADPCM data" );

		acmStreamClose( stream, 0 );
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		FreeData();
		return FALSE;
	}

	// allocate memory to read the wavfile in
	u8 *pSrcData = (u8 *)fang_MallocAndZero( rWavFile.m_dwSize, 4 );
	if( !pSrcData ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "Could not allocate memory to hold our source wav data" );

		acmStreamClose( stream, 0 );
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		FreeData();
		return FALSE;
	}
	u32 nSizeRead;
	if( !rWavFile.Read( (unsigned char *)pSrcData, rWavFile.m_dwSize, (unsigned long *)&nSizeRead ) ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		rErrorLog.WriteErrorLine( "Could not read the wav file into our src buffer" );

		acmStreamClose( stream, 0 );
		fang_Free( pSrcData );
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		FreeData();
		return FALSE;
	}
	FASSERT( nSizeRead == rWavFile.m_dwSize );

	ACMSTREAMHEADER sh;
	memset( &sh, 0, sizeof( ACMSTREAMHEADER ) );
	sh.cbStruct = sizeof( ACMSTREAMHEADER );
	sh.pbSrc = (LPBYTE)pSrcData;
	sh.cbSrcLength = rWavFile.m_dwSize;
	sh.pbDst = (unsigned char *)m_pData;
	sh.cbDstLength = m_nDataSize;
	
	r = acmStreamPrepareHeader( stream, &sh, 0 );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmStreamPrepareHeader failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );

		acmStreamClose( stream, 0 );
		fang_Free( pSrcData );
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		FreeData();
		return FALSE;
	}

	r = acmStreamConvert( stream, &sh, ACM_STREAMCONVERTF_START );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmStreamConvert failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );

		acmStreamClose( stream, 0 );
		fang_Free( pSrcData );
		acmStreamUnprepareHeader( stream, &sh, 0 );
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		FreeData();
		return FALSE;
	}

	r = acmStreamUnprepareHeader( stream, &sh, 0 );
	if( r != 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + rsWavName );
		sError.Format( "acmStreamUnprepareHeader failed - returned %d", r );
		rErrorLog.WriteErrorLine( sError );

		acmStreamClose( stream, 0 );
		fang_Free( pSrcData );
		acmDriverClose( driver, 0 );
		acmDriverRemove( driver_id, 0 );
		FreeLibrary( library );
		FreeData();
		return FALSE;
	}

	acmStreamClose( stream, 0 );	
	acmDriverClose( driver, 0 );
	acmDriverRemove( driver_id, 0 );
	FreeLibrary( library );
	fang_Free( pSrcData );	

	return TRUE;
}

void CWavToADPCM_XBox::FreeData() {
	
	if( m_pData ) {
		fang_Free( m_pData );
		m_pData = NULL;
	}
	m_nDataSize = 0;
	fang_MemZero( &m_XBWavFormat, sizeof( FDX8Data_WaveFormatEx_t ) );	
}


	
