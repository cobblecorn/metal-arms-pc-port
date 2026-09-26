//////////////////////////////////////////////////////////////////////////////////////
// FScript.h - Basic Script Class for Mettle Arms.
//
// Author: Justin Link
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 10/24/01 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FSCRIPT_H_
#define _FSCRIPT_H_ 1

#include "fang.h"

class CFScript
{
public:
	CFScript();
	~CFScript();

	BOOL LoadFromFile(const char *pszScriptFileName);
	// Used to change and retrieve the data area.  Note that GetDataArea does not merely return a pointer to the data
	//   area, it actually copies the data into the buffer supplied.
	BOOL SetDataArea(const void *pDataArea);
	BOOL GetDataArea(void *pDataArea);
	BOOL Unload();

	BOOL m_bIsLoaded;
	void *m_pProgram;
	char m_szScriptFileName[32];
	u32 m_uDataAreaSize;
private:
	void *m_pDataArea;
};
#endif
