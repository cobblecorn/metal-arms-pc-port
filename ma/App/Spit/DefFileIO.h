//////////////////////////////////////////////////////////////////////////////////////
// DefFileIO.h - 
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
// 09/07/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DEF_FILE_IO_H_
#define _DEF_FILE_IO_H_ 1

#include "fang.h"
#include "fparticle.h"

class CDefFileIO
{
public:
	static BOOL Read( const CString &rsFilename, FParticleDef_t *pDest, BOOL &rbDataWasUpdated );
	static BOOL Write( const CString &rsFilename, FParticleDef_t *pSrc );

private:
	static BOOL ConvertVer1ToVer2( void *pVer1, void *pVer2 );
	static BOOL ConvertVer2ToVer3( void *pVer2, void *pVer3 );
};

#endif

