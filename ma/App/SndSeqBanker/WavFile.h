//////////////////////////////////////////////////////////////////////////////////////
// WavFile.h - "from the directx 8.1 sdk"
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
// 06/08/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WAVFILE_H_
#define _WAVFILE_H_ 1

#include "fang.h"
#include <mmsystem.h>
#include <mmreg.h>

#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------
class CWaveFile
{
public:
    WAVEFORMATEX *m_pwfx;// Pointer to WAVEFORMATEX structure
    HMMIO m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO m_ck;       // Multimedia RIFF chunk
    MMCKINFO m_ckRiff;   // Use in opening a WAVE file
    DWORD m_dwSize;      // The size of the wave file
    MMIOINFO m_mmioinfoOut;
    DWORD m_dwFlags;
	f32 m_fSecsOfData;	// how many seconds is the wav file
        
protected:
    BOOL ReadMMIO();
    BOOL WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    CWaveFile();
    ~CWaveFile();

    BOOL Open( LPTSTR strFileName, WAVEFORMATEX *pwfx, DWORD dwFlags );
    BOOL OpenFromMemory( BYTE *pbMemoryFile, ULONG ulDataSize, DWORD dwFlags );
    BOOL Close();

    BOOL Read( BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead );
    BOOL Write( UINT nSizeToWrite, BYTE *pbData, UINT *pnSizeWrote );

    DWORD GetSize();
    BOOL ResetFile();
    WAVEFORMATEX *GetFormat() { return m_pwfx; };
};


#endif

