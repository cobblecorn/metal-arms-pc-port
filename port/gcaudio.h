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
