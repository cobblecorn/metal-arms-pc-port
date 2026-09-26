//////////////////////////////////////////////////////////////////////////////////////
// WavToADPCM_Xbox.h - 
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
#ifndef _WAV_TO_ADPCM_XBOX_H_
#define _WAV_TO_ADPCM_XBOX_H_ 1

#include "fang.h"
#include "WavFile.h"
#include "fdata.h"


class CWavToADPCM_XBox
{
public:
	CWavToADPCM_XBox();
	~CWavToADPCM_XBox();

	BOOL ConvertWavFile( CWaveFile &rWavFile, const CString &rsWavName );

	u8 *m_pData;
	u32 m_nDataSize;
	FDX8Data_WaveFormatEx_t m_XBWavFormat;
	
private:
	void FreeData();

};

#endif

