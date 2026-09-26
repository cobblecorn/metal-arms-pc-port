//////////////////////////////////////////////////////////////////////////////////////
// GCBank.h - 
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
// 06/14/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _GC_BANK_H_
#define _GC_BANK_H_ 1

#include "fang.h"
#include "WavBank.h"
#include "ParseHeader.h"

class CGCBank {
public:
	CGCBank();
	~CGCBank();
	BOOL CreateGCBank( CWavBank &rWavBank,
					   CParseHeader &rHeader,
					   CString &rsBankName,
					   CString &rsOutputFilename );

};

#endif

