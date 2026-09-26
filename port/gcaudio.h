// GameCube MusyX sound data for the Windows DirectSound audio path.
#pragma once

#include "fang.h"
#include "fdata.h"

// Loads snd_init.rdg (MusyX project, pool and sample directory), indexes each sound
// effect ID to its DSP-ADPCM sample, and opens snd_smpls.rdg for sample reads.
BOOL gcaudio_Init( void );
void gcaudio_Uninit( void );

// Converts a retail GameCube wave bank (.rdg: big-endian FDataWvbFile_Bank_t, waves, MusyX
// group ID and one sound effect ID per wave) into the layout the Windows audio code keeps
// for a loaded bank: bank, waves, one PCM FDX8Data_WaveFormatEx_t per wave, then 16-bit PCM.
// The result is allocated with fres_Alloc. Returns NULL if the bank is not valid.
FDataWvbFile_Bank_t *gcaudio_ConvertBank( const void *pData, u32 nBytes, cchar *pszResName );

// A retail GameCube stream (.wvs: music and speech), mono or stereo DSP-ADPCM.
typedef struct {
	u32 nChannels;
	u32 nRate;
	u32 nSamplesPerChannel;
	u32 nBytesPerChannel;	// ADPCM bytes
	f32 fSeconds;
	s16 anCoefs[2 * 16];	// per channel
} GCAudioStreamInfo_t;

// Reads a .wvs header (the first 96 bytes). Returns FALSE if it is not one this port plays.
BOOL gcaudio_ReadStreamHeader( const void *pHeader, u32 nHeaderBytes, u32 nFileBytes, GCAudioStreamInfo_t *pInfo );

// Decodes a whole .wvs file into interleaved 16-bit PCM (nChannels * nSamplesPerChannel
// samples). Safe to call on a worker thread; stops early and returns FALSE once *pnCancel is set.
BOOL gcaudio_DecodeStream( const void *pFile, u32 nFileBytes, const GCAudioStreamInfo_t *pInfo, s16 *pnOut, volatile long *pnCancel );
