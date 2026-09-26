/*---------------------------------------------------------------------------*
  Project:  GAMECUBE DSP ADPCM win32 DLL
  File:     dspadpcm.h

  Copyright 1998, 1999, 2000, 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/

#ifndef __DSPADPCM_H__
#define __DSPADPCM_H__

//#define MAKE_A_DLL

#ifdef MAKE_A_DLL
#define LINKDLL __declspec(dllexport)
#else
#define LINKDLL __declspec(dllimport)
#endif

/*---------------------------------------------------------------------------*
    ADPCM info passed to the caller
 *---------------------------------------------------------------------------*/
typedef struct
{
    // start context
    s16 coef[16];
    u16 gain;
    u16 pred_scale;
    s16 yn1;
    s16 yn2;

    // loop context
    u16 loop_pred_scale;
    s16 loop_yn1;
    s16 loop_yn2;

} ADPCMINFO;


/*---------------------------------------------------------------------------*
    exported functions
 *---------------------------------------------------------------------------*/
/*
LINKDLL u32 getBytesForAdpcmBuffer      (u32 samples);
LINKDLL u32 getBytesForAdpcmSamples     (u32 samples);
LINKDLL u32 getBytesForPcmBuffer        (u32 samples);
LINKDLL u32 getBytesForPcmSamples       (u32 samples);
LINKDLL u32 getSampleForAdpcmNibble     (u32 nibble);
LINKDLL u32 getBytesForAdpcmInfo        (void);
LINKDLL u32 getNibbleAddress            (u32 samples);

LINKDLL void encode
(
    s16         *src,   // location of source samples (16bit PCM signed little endian)
    u8          *dst,   // location of destination buffer
    ADPCMINFO   *cxt,   // location of adpcm info
    u32         samples // number of samples to encode         
);

LINKDLL void decode
(
    u8          *src,   // location of encoded source samples
    s16         *dst,   // location of destination buffer (16 bits / sample)
    ADPCMINFO   *cxt,   // location of adpcm info
    u32         samples // number of samples to decode         
);

LINKDLL void getLoopContext
(
    u8          *src,      // location of ADPCM buffer in RAM
    ADPCMINFO   *cxt,      // location of adpcminfo
    u32         samples    // samples to desired context
);
*/
#endif