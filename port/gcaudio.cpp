// GameCube MusyX sound data for the Windows DirectSound audio path.
//
// The retail GameCube game plays wave banks through MusyX. Each bank (.rdg) lists its waves
// and one MusyX sound effect ID per wave. snd_init.rdg holds the MusyX project (per group, a
// table from sound effect ID to SoundMacro), the pool (SoundMacros, whose StartSample command
// names a sample) and the sample directory (offset, rate, length and DSP-ADPCM coefficients
// for each sample in snd_smpls.rdg). Across the retail data every wave is a mono DSP-ADPCM
// sample at its recorded rate (base note 60, no transposition), which is what this converts.
//
// Offsets and counts are validated before use; the data is big-endian.

#include "gcaudio.h"
#include "fdx8data.h"
#include "ffile.h"
#include "fres.h"
#include "fclib.h"
#include "fmath.h"

#include <stdlib.h>
#include <string.h>

namespace {

struct Sample {
	u32 nOffset;		// Byte offset of the first ADPCM frame in snd_smpls.rdg
	u32 nSamples;
	u32 nRate;
	s16 anCoefs[16];	// Eight predictor pairs
};

const u32 _SFX_ID_COUNT = 0x10000;
const u16 _NO_SAMPLE = 0xFFFF;

Sample *_paSamples;			// Indexed by MusyX sample ID
u32 _nSampleIdCount;
u16 *_panSfxToSample;		// Indexed by MusyX sound effect ID
FFileHandle _hSamples = FFILE_INVALID_HANDLE;
u32 _nSamplesFileBytes;

u16 _BE16( const u8 *p ) { return (u16)( (p[0] << 8) | p[1] ); }
u32 _BE32( const u8 *p ) { return ( (u32)p[0] << 24 ) | ( (u32)p[1] << 16 ) | ( (u32)p[2] << 8 ) | p[3]; }
f32 _BEF32( const u8 *p ) { u32 n = _BE32( p ); f32 f; memcpy( &f, &n, sizeof(f) ); return f; }
BOOL _InRange( u32 nOffset, u32 nBytes, u32 nTotal ) { return nOffset <= nTotal && nBytes <= nTotal - nOffset; }

BOOL _ReadWholeFile( cchar *pszName, u8 **ppData, u32 *pnBytes ) {
	FFileHandle hFile = ffile_Open( pszName, FFILE_OPEN_RONLY );
	if( !FFILE_IS_VALID_HANDLE( hFile ) ) {
		return FALSE;
	}
	const s32 nBytes = ffile_GetFileSize( hFile );
	u8 *pData = nBytes > 0 ? (u8 *)malloc( nBytes ) : NULL;
	const BOOL bOK = pData && ffile_Read( hFile, nBytes, pData ) == nBytes;
	ffile_Close( hFile );
	if( !bOK ) {
		free( pData );
		return FALSE;
	}
	*ppData = pData;
	*pnBytes = (u32)nBytes;
	return TRUE;
}

// Fills pnMacroToSample from the pool's SoundMacros: the first StartSample (opcode 0x10 in
// the low byte of a command's first word) gives the macro's sample.
BOOL _IndexMacros( const u8 *pPool, u32 nPoolBytes, u16 *pnMacroToSample ) {
	if( nPoolBytes < 16 ) return FALSE;
	u32 nOffset = _BE32( pPool );
	while( _InRange( nOffset, 4, nPoolBytes ) ) {
		const u32 nSize = _BE32( pPool + nOffset );
		if( nSize == 0xFFFFFFFF ) return TRUE;
		if( nSize < 8 || !_InRange( nOffset, nSize, nPoolBytes ) ) return FALSE;
		const u16 nMacro = _BE16( pPool + nOffset + 4 );
		for( u32 nCmd = nOffset + 8; nCmd + 8 <= nOffset + nSize; nCmd += 8 ) {
			const u32 nWord = _BE32( pPool + nCmd );
			if( (nWord & 0xFF) == 0x10 ) {
				pnMacroToSample[nMacro] = (u16)( (nWord >> 8) & 0xFFFF );
				break;
			}
		}
		nOffset += nSize;
	}
	return FALSE;
}

// Fills pnSfxToMacro from every sound effect group's table in the project.
BOOL _IndexSfxTables( const u8 *pProj, u32 nProjBytes, u16 *pnSfxToMacro ) {
	u32 nOffset = 0;
	for( u32 nGroups = 0; nGroups < 4096 && _InRange( nOffset, 4, nProjBytes ); ++nGroups ) {
		const u32 nEnd = _BE32( pProj + nOffset );
		if( nEnd == 0xFFFFFFFF ) return TRUE;
		if( !_InRange( nOffset, 40, nProjBytes ) || nEnd <= nOffset || nEnd > nProjBytes ) return FALSE;
		if( _BE16( pProj + nOffset + 6 ) == 1 ) {	// sound effect group
			const u32 nTable = _BE32( pProj + nOffset + 28 );	// offsets are relative to the project
			if( !_InRange( nTable, 4, nProjBytes ) ) return FALSE;
			const u32 nCount = _BE16( pProj + nTable );
			if( !_InRange( nTable + 4, nCount * 10, nProjBytes ) ) return FALSE;
			for( u32 i = 0; i < nCount; ++i ) {
				const u8 *pEntry = pProj + nTable + 4 + i * 10;
				pnSfxToMacro[_BE16( pEntry )] = _BE16( pEntry + 2 );
			}
		}
		nOffset = nEnd;
	}
	return FALSE;
}

// Reads the sample directory: 32-byte entries ending with ID 0xFFFF, each pointing at a
// 40-byte DSP-ADPCM record (bytes per frame, predictor/scale, history, coefficients).
BOOL _IndexSamples( const u8 *pSdir, u32 nSdirBytes ) {
	u32 nMaxId = 0, nEntries = 0;
	for( u32 nOffset = 0; _InRange( nOffset, 32, nSdirBytes ) && _BE16( pSdir + nOffset ) != 0xFFFF; nOffset += 32 ) {
		nMaxId = FMATH_MAX( nMaxId, (u32)_BE16( pSdir + nOffset ) );
		++nEntries;
	}
	if( !nEntries ) return FALSE;

	_nSampleIdCount = nMaxId + 1;
	_paSamples = (Sample *)calloc( _nSampleIdCount, sizeof(Sample) );
	if( !_paSamples ) return FALSE;

	u32 nRejected = 0;
	for( u32 i = 0; i < nEntries; ++i ) {
		const u8 *pEntry = pSdir + i * 32;
		const u32 nLength = _BE32( pEntry + 16 );
		const u32 nSamples = nLength & 0xFFFFFF, nFormat = nLength >> 24;
		const u32 nOffset = _BE32( pEntry + 4 ), nRate = _BE16( pEntry + 14 ), nInfo = _BE32( pEntry + 28 );
		const u32 nFrameBytes = ( (nSamples + 13) / 14 ) * 8;
		if( nFormat != 0 || !nSamples || nRate < 4000 || nRate > 48000 || _BE16( pEntry + 2 ) != 0 ||
			!_InRange( nOffset, nFrameBytes, _nSamplesFileBytes ) || !_InRange( nInfo, 40, nSdirBytes ) ||
			_BE16( pSdir + nInfo ) != 8 ) {
			++nRejected;
			continue;
		}
		Sample &rSample = _paSamples[_BE16( pEntry )];
		rSample.nOffset = nOffset;
		rSample.nSamples = nSamples;
		rSample.nRate = nRate;
		for( u32 j = 0; j < 16; ++j ) {
			rSample.anCoefs[j] = (s16)_BE16( pSdir + nInfo + 8 + j * 2 );
		}
	}
	if( nRejected ) {
		DEVPRINTF( "gcaudio: %d MusyX samples use an unsupported format and will play silence.\n", nRejected );
	}
	return TRUE;
}

// Decodes GameCube DSP-ADPCM (8-byte frames of 14 samples) to 16-bit PCM, writing every
// nStride'th output sample. pnHist carries the two previous samples across calls.
void _DecodeDspAdpcm( const u8 *pFrames, u32 nSamples, const s16 *pnCoefs, s16 *pnOut, u32 nStride, s32 *pnHist ) {
	s32 nHist1 = pnHist[0], nHist2 = pnHist[1];
	for( u32 nDone = 0; nDone < nSamples; pFrames += 8 ) {
		const u32 nHeader = pFrames[0];
		const s32 nScale = 1 << (nHeader & 0xF);
		const u32 nPredictor = (nHeader >> 4) & 7;
		const s32 nCoef1 = pnCoefs[nPredictor * 2], nCoef2 = pnCoefs[nPredictor * 2 + 1];
		for( u32 i = 0; i < 14 && nDone < nSamples; ++i, ++nDone ) {
			s32 nNibble = ( i & 1 ) ? ( pFrames[1 + i / 2] & 0xF ) : ( pFrames[1 + i / 2] >> 4 );
			if( nNibble >= 8 ) nNibble -= 16;
			s32 nSample = ( ( (nNibble * nScale) << 11 ) + 1024 + nCoef1 * nHist1 + nCoef2 * nHist2 ) >> 11;
			FMATH_CLAMP( nSample, -32768, 32767 );
			pnOut[nDone * nStride] = (s16)nSample;
			nHist2 = nHist1;
			nHist1 = nSample;
		}
	}
	pnHist[0] = nHist1;
	pnHist[1] = nHist2;
}

// .wvs stream header (FGCData_WvsFile_Header_t): six u32/f32 fields, then 16 coefficients
// per channel for two channels, padded to 96 bytes. Channel data follows in chunks of
// nChunkBytes, interleaved by channel; the last chunk of each channel may be shorter.
const u32 _WVS_HEADER_BYTES = 96;
const u32 _WVS_CHUNK_BYTES = 4096;

} // namespace


BOOL gcaudio_Init( void ) {
	gcaudio_Uninit();

	_hSamples = ffile_Open( "snd_smpls.rdg", FFILE_OPEN_RONLY );
	if( !FFILE_IS_VALID_HANDLE( _hSamples ) || ffile_GetFileSize( _hSamples ) <= 0 ) {
		DEVPRINTF( "gcaudio: could not open snd_smpls.rdg.\n" );
		gcaudio_Uninit();
		return FALSE;
	}
	_nSamplesFileBytes = (u32)ffile_GetFileSize( _hSamples );

	u8 *pInit = NULL;
	u32 nInitBytes = 0;
	if( !_ReadWholeFile( "snd_init.rdg", &pInit, &nInitBytes ) || nInitBytes < 24 ) {
		DEVPRINTF( "gcaudio: could not read snd_init.rdg.\n" );
		free( pInit );
		gcaudio_Uninit();
		return FALSE;
	}

	const u32 nProjBytes = _BE32( pInit ), nProjOffset = _BE32( pInit + 4 );
	const u32 nPoolBytes = _BE32( pInit + 8 ), nPoolOffset = _BE32( pInit + 12 );
	const u32 nSdirBytes = _BE32( pInit + 16 ), nSdirOffset = _BE32( pInit + 20 );
	u16 *pnSfxToMacro = (u16 *)malloc( _SFX_ID_COUNT * sizeof(u16) );
	u16 *pnMacroToSample = (u16 *)malloc( _SFX_ID_COUNT * sizeof(u16) );
	_panSfxToSample = (u16 *)malloc( _SFX_ID_COUNT * sizeof(u16) );
	BOOL bOK = pnSfxToMacro && pnMacroToSample && _panSfxToSample &&
		_InRange( nProjOffset, nProjBytes, nInitBytes ) && _InRange( nPoolOffset, nPoolBytes, nInitBytes ) &&
		_InRange( nSdirOffset, nSdirBytes, nInitBytes );
	if( bOK ) {
		memset( pnSfxToMacro, 0xFF, _SFX_ID_COUNT * sizeof(u16) );
		memset( pnMacroToSample, 0xFF, _SFX_ID_COUNT * sizeof(u16) );
		memset( _panSfxToSample, 0xFF, _SFX_ID_COUNT * sizeof(u16) );
		bOK = _IndexSfxTables( pInit + nProjOffset, nProjBytes, pnSfxToMacro ) &&
			_IndexMacros( pInit + nPoolOffset, nPoolBytes, pnMacroToSample ) &&
			_IndexSamples( pInit + nSdirOffset, nSdirBytes );
	}

	u32 nSounds = 0;
	for( u32 nSfx = 0; bOK && nSfx < _SFX_ID_COUNT; ++nSfx ) {
		const u16 nMacro = pnSfxToMacro[nSfx];
		const u16 nSample = nMacro != 0xFFFF ? pnMacroToSample[nMacro] : _NO_SAMPLE;
		if( nSample < _nSampleIdCount && _paSamples[nSample].nSamples ) {
			_panSfxToSample[nSfx] = nSample;
			++nSounds;
		}
	}

	free( pnSfxToMacro );
	free( pnMacroToSample );
	free( pInit );
	if( !bOK ) {
		DEVPRINTF( "gcaudio: snd_init.rdg is not a MusyX data file this port understands.\n" );
		gcaudio_Uninit();
		return FALSE;
	}
	DEVPRINTF( "gcaudio: %d MusyX sound effects map to DSP-ADPCM samples.\n", nSounds );
	return TRUE;
}


void gcaudio_Uninit( void ) {
	if( FFILE_IS_VALID_HANDLE( _hSamples ) ) {
		ffile_Close( _hSamples );
	}
	_hSamples = FFILE_INVALID_HANDLE;
	_nSamplesFileBytes = 0;
	free( _paSamples );
	free( _panSfxToSample );
	_paSamples = NULL;
	_panSfxToSample = NULL;
	_nSampleIdCount = 0;
}


FDataWvbFile_Bank_t *gcaudio_ConvertBank( const void *pData, u32 nBytes, cchar *pszResName ) {
	FASSERT( sizeof(FDataWvbFile_Bank_t) == 32 && sizeof(FDataWvbFile_Wave_t) == 40 );
	const u8 *pSource = (const u8 *)pData;
	if( !_panSfxToSample || !pSource || nBytes < 32 ) {
		return NULL;
	}

	const u32 nWaves = _BE32( pSource + 12 );
	if( !nWaves || nWaves > 4096 || !_InRange( 32, nWaves * 40 + (nWaves + 1) * 4, nBytes ) ) {
		DEVPRINTF( "gcaudio: '%s' is not a GameCube wave bank.\n", pszResName ? pszResName : "(unnamed)" );
		return NULL;
	}
	const u8 *pSourceWaves = pSource + 32;
	const u8 *pSoundIds = pSourceWaves + nWaves * 40 + 4;	// after the MusyX group ID

	// Waves without a usable sample get a short silence so wave indices stay valid.
	const u32 nSilentSamples = 64;
	u32 nPcmBytes = 0;
	for( u32 i = 0; i < nWaves; ++i ) {
		const u32 nSfx = _BE32( pSoundIds + i * 4 );
		const u16 nSample = nSfx < _SFX_ID_COUNT ? _panSfxToSample[nSfx] : _NO_SAMPLE;
		nPcmBytes += 2 * ( nSample != _NO_SAMPLE ? _paSamples[nSample].nSamples : nSilentSamples );
	}

	const u32 nFormatBytes = nWaves * sizeof(FDX8Data_WaveFormatEx_t);
	const u32 nTotal = sizeof(FDataWvbFile_Bank_t) + nWaves * sizeof(FDataWvbFile_Wave_t) + nFormatBytes + nPcmBytes;
	FDataWvbFile_Bank_t *pBank = (FDataWvbFile_Bank_t *)fres_AlignedAllocAndZero( nTotal, 16 );
	if( !pBank ) {
		DEVPRINTF( "gcaudio: not enough memory for the %d bytes of wave bank '%s'.\n", nTotal, pszResName ? pszResName : "(unnamed)" );
		return NULL;
	}

	fclib_strncpy( pBank->szName, (cchar *)pSource, FDATA_AUDIO_FILENAME_LEN - 1 );
	pBank->uWaves = nWaves;
	FDataWvbFile_Wave_t *paWaves = (FDataWvbFile_Wave_t *)&pBank[1];
	pBank->pData = &paWaves[nWaves];
	pBank->uDataLength = nFormatBytes + nPcmBytes;
	FDX8Data_WaveFormatEx_t *paFormats = (FDX8Data_WaveFormatEx_t *)pBank->pData;
	u8 *pPcm = (u8 *)pBank->pData + nFormatBytes;

	u8 *pAdpcm = NULL;
	u32 nAdpcmCapacity = 0, nPcmOffset = 0, nMissing = 0;
	for( u32 i = 0; i < nWaves; ++i ) {
		const u8 *pSourceWave = pSourceWaves + i * 40;
		const u32 nSfx = _BE32( pSoundIds + i * 4 );
		const u16 nSample = nSfx < _SFX_ID_COUNT ? _panSfxToSample[nSfx] : _NO_SAMPLE;
		const Sample *pSample = nSample != _NO_SAMPLE ? &_paSamples[nSample] : NULL;
		const u32 nSamples = pSample ? pSample->nSamples : nSilentSamples;
		const u32 nRate = pSample ? pSample->nRate : 22050;

		FDataWvbFile_Wave_t &rWave = paWaves[i];
		fclib_strncpy( rWave.szName, (cchar *)pSourceWave, FDATA_AUDIO_FILENAME_LEN - 1 );
		rWave.uLength = nSamples * 2;
		rWave.uOffset = nPcmOffset;
		rWave.oBankHandle = (FAudio_BankHandle_t)pBank;
		rWave.uIndex = i;
		rWave.fLengthInSeconds = (f32)nSamples / (f32)nRate;
		rWave.nNumChannels = 1;
		rWave.nFlags = _BE16( pSourceWave + 34 );
		rWave.fFreqHz = (f32)nRate;

		WAVEFORMATEX &rFormat = paFormats[i].WaveFormatEx;
		rFormat.wFormatTag = FDX8DATA_WAVEFORMATEX_ID_PCM;
		rFormat.nChannels = 1;
		rFormat.nSamplesPerSec = nRate;
		rFormat.wBitsPerSample = 16;
		rFormat.nBlockAlign = 2;
		rFormat.nAvgBytesPerSec = nRate * 2;
		rFormat.cbSize = 0;

		if( pSample ) {
			const u32 nFrameBytes = ( (nSamples + 13) / 14 ) * 8;
			if( nFrameBytes > nAdpcmCapacity ) {
				free( pAdpcm );
				nAdpcmCapacity = nFrameBytes;
				pAdpcm = (u8 *)malloc( nAdpcmCapacity );
			}
			if( pAdpcm && ffile_Seek( _hSamples, (s32)pSample->nOffset, FFILE_SEEK_SET ) >= 0 &&
				ffile_Read( _hSamples, nFrameBytes, pAdpcm ) == (s32)nFrameBytes ) {
				s32 anHist[2] = { 0, 0 };
				_DecodeDspAdpcm( pAdpcm, nSamples, pSample->anCoefs, (s16 *)( pPcm + nPcmOffset ), 1, anHist );
			} else {
				++nMissing;		// left as zeroed silence
			}
		} else {
			++nMissing;
		}
		nPcmOffset += nSamples * 2;
	}
	free( pAdpcm );

	if( nMissing ) {
		DEVPRINTF( "gcaudio: %d of %d waves in '%s' have no GameCube sample and are silent.\n", nMissing, nWaves, pszResName ? pszResName : "(unnamed)" );
	}
	return pBank;
}


BOOL gcaudio_ReadStreamHeader( const void *pHeader, u32 nHeaderBytes, u32 nFileBytes, GCAudioStreamInfo_t *pInfo ) {
	const u8 *pBytes = (const u8 *)pHeader;
	if( !pBytes || !pInfo || nHeaderBytes < _WVS_HEADER_BYTES ) {
		return FALSE;
	}
	const u32 nChannels = _BE32( pBytes );
	const f32 fSeconds = _BEF32( pBytes + 4 ), fRate = _BEF32( pBytes + 8 );
	const u32 nChunkBytes = _BE32( pBytes + 12 ), nChunks = _BE32( pBytes + 16 ), nBytesPerChannel = _BE32( pBytes + 20 );
	if( nChannels < 1 || nChannels > 2 || nChunkBytes != _WVS_CHUNK_BYTES || !( fRate >= 4000.0f && fRate <= 48000.0f ) ||
		!( fSeconds >= 0.0f ) || !nBytesPerChannel || nChunks != ( nBytesPerChannel + nChunkBytes - 1 ) / nChunkBytes ||
		!_InRange( _WVS_HEADER_BYTES, nChannels * nBytesPerChannel, nFileBytes ) ) {
		return FALSE;
	}
	pInfo->nChannels = nChannels;
	pInfo->nRate = (u32)fRate;
	pInfo->nSamplesPerChannel = ( nBytesPerChannel / 8 ) * 14;
	pInfo->nBytesPerChannel = nBytesPerChannel;
	pInfo->fSeconds = (f32)pInfo->nSamplesPerChannel / fRate;
	for( u32 i = 0; i < 2 * 16; ++i ) {
		pInfo->anCoefs[i] = (s16)_BE16( pBytes + 24 + i * 2 );
	}
	return TRUE;
}


BOOL gcaudio_DecodeStream( const void *pFile, u32 nFileBytes, const GCAudioStreamInfo_t *pInfo, s16 *pnOut, volatile long *pnCancel ) {
	const u8 *pBytes = (const u8 *)pFile;
	if( !pBytes || !pInfo || !pnOut || !_InRange( _WVS_HEADER_BYTES, pInfo->nChannels * pInfo->nBytesPerChannel, nFileBytes ) ) {
		return FALSE;
	}
	const u32 nChannels = pInfo->nChannels;
	s32 aanHist[2][2] = { { 0, 0 }, { 0, 0 } };
	u32 nOffset = _WVS_HEADER_BYTES, nRemaining = pInfo->nBytesPerChannel, nSamplesDone = 0;
	while( nRemaining >= 8 ) {
		if( pnCancel && *pnCancel ) {
			return FALSE;
		}
		const u32 nChunk = FMATH_MIN( nRemaining, _WVS_CHUNK_BYTES );
		const u32 nSamples = ( nChunk / 8 ) * 14;
		for( u32 c = 0; c < nChannels; ++c ) {
			_DecodeDspAdpcm( pBytes + nOffset + c * nChunk, nSamples, &pInfo->anCoefs[c * 16],
				pnOut + nSamplesDone * nChannels + c, nChannels, aanHist[c] );
		}
		nSamplesDone += nSamples;
		nOffset += nChannels * nChunk;
		nRemaining -= nChunk;
	}
	return TRUE;
}
