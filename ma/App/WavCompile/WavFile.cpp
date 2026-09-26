//////////////////////////////////////////////////////////////////////////////////////
// WavFile.cpp - "from the directx 8.1 sdk"
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
#include "stdafx.h"
#include "fang.h"
#include "WavFile.h"


//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }


//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.  
//       Then call Read() as needed.  Calling the destructor or Close() 
//       will close the file.  
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile() {
    m_pwfx    = NULL;
    m_hmmio   = NULL;
    m_dwSize  = 0;
	m_fSecsOfData = 0.0f;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile() {
    Close();

    SAFE_DELETE_ARRAY( m_pwfx );
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
BOOL CWaveFile::Open( LPTSTR strFileName, WAVEFORMATEX *pwfx, DWORD dwFlags ) {
    HRESULT hr;

    m_dwFlags = dwFlags;
    
    if( m_dwFlags == WAVEFILE_READ ) {
        if( strFileName == NULL ) {
            return FALSE;
		}
        SAFE_DELETE_ARRAY( m_pwfx );

        m_hmmio = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ );

        if( NULL == m_hmmio ) {
            return FALSE;
        }

        if( FAILED( hr = ReadMMIO() ) ) {
            // ReadMMIO will fail if its an not a wave file
            mmioClose( m_hmmio, 0 );
            return FALSE;
        }

        if( FAILED( hr = ResetFile() ) ) {
            return FALSE;
		}

        // After the reset, the size of the wav file is m_ck.cksize so store it now
        m_dwSize = m_ck.cksize;

		m_fSecsOfData = (f32)m_dwSize / (f32)m_pwfx->nAvgBytesPerSec;
    } else {
        m_hmmio = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF  | MMIO_READWRITE | MMIO_CREATE );
        if( NULL == m_hmmio ) {
            return FALSE;
		}

        if( FAILED( hr = WriteMMIO( pwfx ) ) ) {
            mmioClose( m_hmmio, 0 );
            return FALSE;
        }
                        
        if( FAILED( hr = ResetFile() ) ) {
            return FALSE;
		}
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::OpenFromMemory()
// Desc: Opens a memory file for reading
//-----------------------------------------------------------------------------
BOOL CWaveFile::OpenFromMemory( BYTE *pbMemoryFile, ULONG ulDataSize, DWORD dwFlags ) {
	HRESULT hr;

	if( dwFlags != WAVEFILE_READ ) {
        return FALSE;       
	}

	m_dwFlags = dwFlags;
    
    if( m_dwFlags == WAVEFILE_READ ) {
        SAFE_DELETE_ARRAY( m_pwfx );

		MMIOINFO MMIOInfo;
		fang_MemZero( &MMIOInfo, sizeof( MMIOINFO ) );
		MMIOInfo.pchBuffer = (char *)pbMemoryFile;
		MMIOInfo.fccIOProc  = FOURCC_MEM;
		MMIOInfo.cchBuffer = ulDataSize;
		
        m_hmmio = mmioOpen( NULL, &MMIOInfo, MMIO_READ );

        if( NULL == m_hmmio ) {
            return FALSE;
        }

        if( FAILED( hr = ReadMMIO() ) ) {
            // ReadMMIO will fail if its an not a wave file
            mmioClose( m_hmmio, 0 );
            return FALSE;
        }

        if( FAILED( hr = ResetFile() ) ) {
            return FALSE;
		}

        // After the reset, the size of the wav file is m_ck.cksize so store it now
        m_dwSize = m_ck.cksize;

		m_fSecsOfData = (f32)m_dwSize / (f32)m_pwfx->nAvgBytesPerSec;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx. 
//-----------------------------------------------------------------------------
BOOL CWaveFile::ReadMMIO() {
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       

    m_pwfx = NULL;

    if( ( 0 != mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) ) {
        return FALSE;
	}

    // Check to make sure this is a valid wave file
    if( (m_ckRiff.ckid != FOURCC_RIFF) ||
        (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) ) {
        return FALSE; 
	}

    // Search the input file for for the 'fmt ' chunk.
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) ) {
        return FALSE;
	}

    // Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
    // if there are extra parameters at the end, we'll ignore them
	if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) ) {
		return FALSE;
	}

    // Read the 'fmt ' chunk into <pcmWaveFormat>.
    if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) ) {
        return FALSE;
	}

    // Allocate the waveformatex, but if its not pcm format, read the next
    // word, and thats how many extra bytes to allocate.
    if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM ) {
        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) ];
        if( NULL == m_pwfx ) {
            return FALSE;
		}

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = 0;
    } else {
        // Read in length of extra bytes.
        WORD cbExtraBytes = 0L;
        if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) ) {
            return FALSE;
		}

        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
        if( NULL == m_pwfx ) {
            return FALSE;
		}

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = cbExtraBytes;

        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
        if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)), cbExtraBytes ) != cbExtraBytes ) {
            SAFE_DELETE( m_pwfx );
            return FALSE;
        }
    }

    // Ascend the input file out of the 'fmt ' chunk.
    if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) ) {
        SAFE_DELETE( m_pwfx );
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::GetSize()
// Desc: Retuns the size of the read access wave file 
//-----------------------------------------------------------------------------
DWORD CWaveFile::GetSize() {
    return m_dwSize;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the 
//       beginning of the file again 
//-----------------------------------------------------------------------------
BOOL CWaveFile::ResetFile() {

    if( m_hmmio == NULL ) {
        return FALSE;
	}

    if( m_dwFlags == WAVEFILE_READ ) {
        // Seek to the data
        if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET ) ) {
            return FALSE;
		}

        // Search the input file for the 'data' chunk.
        m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
        if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) ) {
			return FALSE;
		}
    } else {
        // Create the 'data' chunk that holds the waveform samples.  
        m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
        m_ck.cksize = 0;

        if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) ) {
            return FALSE;
		}

        if( 0 != mmioGetInfo( m_hmmio, &m_mmioinfoOut, 0 ) ) {
            return FALSE;
		}
    }
    
    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns 
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So 
//       subsequent calls will be continue where the last left off unless 
//       Reset() is called.
//-----------------------------------------------------------------------------
BOOL CWaveFile::Read( BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead ) {

    MMIOINFO mmioinfoIn; // current status of m_hmmio

    if( m_hmmio == NULL ) {
        return FALSE;
	}
    if( pBuffer == NULL || pdwSizeRead == NULL ) {
        return FALSE;
	}

    if( pdwSizeRead != NULL ) {
        *pdwSizeRead = 0;
	}

    if( 0 != mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) ) {
        return FALSE;
	}
            
    UINT cbDataIn = dwSizeToRead;
    if( cbDataIn > m_ck.cksize ) {
        cbDataIn = m_ck.cksize;   
	}

    m_ck.cksize -= cbDataIn;

    for( DWORD cT = 0; cT < cbDataIn; cT++ ) {
        // Copy the bytes from the io to the buffer.
        if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead ) {
            if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) ) {
                return FALSE;
			}

            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead ) {
                return FALSE;
			}
        }

        // Actual copy.
        *((BYTE*)pBuffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
        mmioinfoIn.pchNext++;
    }

    if( 0 != mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) ) {
        return FALSE;
	}

    if( pdwSizeRead != NULL ) {
        *pdwSizeRead = cbDataIn;
	}

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Close()
// Desc: Closes the wave file 
//-----------------------------------------------------------------------------
BOOL CWaveFile::Close() {

    if( m_dwFlags == WAVEFILE_READ ) {
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;        
    } else {
        m_mmioinfoOut.dwFlags |= MMIO_DIRTY;

        if( m_hmmio == NULL ) {
            return FALSE;
		}

        if( 0 != mmioSetInfo( m_hmmio, &m_mmioinfoOut, 0 ) ) {
            return FALSE;
		}
    
        // Ascend the output file out of the 'data' chunk -- this will cause
        // the chunk size of the 'data' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) ) {
            return FALSE;
		}
    
        // Do this here instead...
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) ) {
            return FALSE;
		}
        
        mmioSeek( m_hmmio, 0, SEEK_SET );

        if( 0 != (INT)mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) {
            return FALSE;
		}
    
        m_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

        if( 0 == mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) ) {
            DWORD dwSamples = 0;
            mmioWrite( m_hmmio, (HPSTR)&dwSamples, sizeof(DWORD) );
            mmioAscend( m_hmmio, &m_ck, 0 ); 
        }
    
        // Ascend the output file out of the 'RIFF' chunk -- this will cause
        // the chunk size of the 'RIFF' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) ) {
            return FALSE;
		}
    
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::WriteMMIO()
// Desc: Support function for reading from a multimedia I/O stream
//       pwfxDest is the WAVEFORMATEX for this new wave file.  
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_ck.  
//-----------------------------------------------------------------------------
BOOL CWaveFile::WriteMMIO( WAVEFORMATEX *pwfxDest ) {
    DWORD    dwFactChunk; // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
    MMCKINFO ckOut1;
    
    dwFactChunk = (DWORD)-1;

    // Create the output file RIFF chunk of form type 'WAVE'.
    m_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');       
    m_ckRiff.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &m_ckRiff, MMIO_CREATERIFF ) ) {
        return FALSE;
	}
    
    // We are now descended into the 'RIFF' chunk we just created.
    // Now create the 'fmt ' chunk. Since we know the size of this chunk,
    // specify it in the MMCKINFO structure so MMIO doesn't have to seek
    // back and set the chunk size after ascending from the chunk.
    m_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    m_ck.cksize = sizeof(PCMWAVEFORMAT);   

    if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) ) {
        return FALSE;
	}
    
    // Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. 
    if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM ) {
        if( mmioWrite( m_hmmio, (HPSTR) pwfxDest, sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT) ) {
            return FALSE;
		}
    } else {
        // Write the variable length size.
        if( (UINT)mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                             sizeof(*pwfxDest) + pwfxDest->cbSize ) != 
                             ( sizeof(*pwfxDest) + pwfxDest->cbSize ) ) {
            return FALSE;
		}
    }  
    
    // Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
    if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) ) {
        return FALSE;
	}
    
    // Now create the fact chunk, not required for PCM but nice to have.  This is filled
    // in when the close routine is called.
    ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
    ckOut1.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &ckOut1, 0 ) ) {
        return FALSE;
	}
    
    if( mmioWrite( m_hmmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) != sizeof(dwFactChunk) ) {
		return FALSE;
	}
    
    // Now ascend out of the fact chunk...
    if( 0 != mmioAscend( m_hmmio, &ckOut1, 0 ) ) {
        return FALSE;
	}
       
    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Write()
// Desc: Writes data to the open wave file
//-----------------------------------------------------------------------------
BOOL CWaveFile::Write( UINT nSizeToWrite, BYTE *pbSrcData, UINT *pnSizeWrote ) {
    UINT cT;

    if( m_hmmio == NULL ) {
        return FALSE;
	}
    if( pnSizeWrote == NULL || pbSrcData == NULL ) {
        return FALSE;
	}

    *pnSizeWrote = 0;
    
    for( cT = 0; cT < nSizeToWrite; cT++ ) {       
        if( m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndWrite ) {
            m_mmioinfoOut.dwFlags |= MMIO_DIRTY;
            if( 0 != mmioAdvance( m_hmmio, &m_mmioinfoOut, MMIO_WRITE ) ) {
                return FALSE;
			}
        }

        *((BYTE*)m_mmioinfoOut.pchNext) = *((BYTE*)pbSrcData+cT);
        (BYTE*)m_mmioinfoOut.pchNext++;

        (*pnSizeWrote)++;
    }

    return TRUE;
}

