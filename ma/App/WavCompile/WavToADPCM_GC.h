//////////////////////////////////////////////////////////////////////////////////////
// WavToADPCM_GC.h - 
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
#ifndef _WAV_TO_ADPCM_GC_H_
#define _WAV_TO_ADPCM_GC_H_ 1

#include "fang.h"
#include "WavFile.h"
#include "fdata.h"


class CWavToADPCM_GC
{
public:
	CWavToADPCM_GC();
	~CWavToADPCM_GC();

	BOOL ConvertWavFile( CWaveFile &rWavFile, const CString &rsWavName );

	u8 *m_pData;
	u32 m_nDataSize;
	
	
private:
	void FreeData();

	u32 m_nBytesAllocated;
	FGCData_WvsFile_Header_t *m_pHeader;
		
	static void FreeDLL( void );
	static int LoadDll( void );	
};

#endif

