//////////////////////////////////////////////////////////////////////////////////////
// FScriptInst.h - Script Instance Class for Mettle Arms.
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
#ifndef _FSCRIPTINST_H_
#define _FSCRIPTINST_H_ 1

#include "fang.h"
#include "amx.h"
#include "FScript.h"

enum
{
	CFSCRIPTINST_FLAG_DOWORK = (1 << 0)
};

#define FSCRIPT_INST_MAX_STATE_SAVES	( 2 )

class CFScriptInst
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	BOOL Init( CFScript *pScript, void *pObject );
	BOOL InitClone( CFScriptInst *pScriptInst, void *pObject );
	BOOL OnInit();
	BOOL OnEvent(u32 uWhichEvent, u32 uEventData1, u32 uEventData2, u32 uEventData3);
	BOOL Work();
	BOOL OnEnd();
	// Execute is not yet implemented.  It is a more general execution method than the above three.
	BOOL Execute(/*stuff in here*/);

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	BOOL m_bIsInitialized;
	void *m_pObject;
	AMX m_oAMX;
	s32 m_nOnInitIdx, m_nOnEventIdx, m_nWorkIdx, m_nOnEndIdx;
	u64 m_uEventFlags;
	u32 m_uFlags;
	void *m_pDataArea;
	CFScript *m_pScript;
	f32 m_fDisabledCtdn;
	s32 m_hStateSave[FSCRIPT_INST_MAX_STATE_SAVES];	// allows clients to associate a state save data handle with script instances
};

#endif