//////////////////////////////////////////////////////////////////////////////////////
// MusyXData.h - 
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
#ifndef _MUSYX_DATA_H_
#define _MUSYX_DATA_H_ 1

#include "fang.h"
#include "FileInfo.h"


class CMusyXData {
public:
	CMusyXData();
	~CMusyXData();

	BOOL CreateMusyXDataFile( CFileInfo *pProjFile,
							  CFileInfo *pSDirFile,
							  CFileInfo *pPoolFile,
							  CString &rsOutputFilename );
};

#endif

