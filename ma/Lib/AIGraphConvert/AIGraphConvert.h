//////////////////////////////////////////////////////////////////////////////////////
// AIGraphConvert.h - 
//
// Author: Pat MacKellar 
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
// 04/02/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIGRAPHCONVERT_H_
#define _AIGRAPHCONVERT_H_ 1

#include <stdio.h>



#if defined(__cplusplus)
extern "C" {
#endif

unsigned long aigraphconvert_fnAIGraphConvertAsciiToBinary(const char* pszFilePath);
BOOL aigraphconvert_fnAIGraphEndianFlip(unsigned long uHandle);
int aigraphconvert_fnAIGraphCalcConvertedFileSizeBytes(unsigned long uHandle);
BOOL aigraphconvert_fnAIGraphSaveToBuffer(unsigned long uHandle, const char* pszFilename, FILE* pFileStream /*NULL*/);
void aigraphconvert_fvAIGraphFreeData(unsigned long uHandle);

#define AIGRAPHCONVERT_INVALIDHANDLE 0

#if defined(__cplusplus)
}
#endif



#endif


