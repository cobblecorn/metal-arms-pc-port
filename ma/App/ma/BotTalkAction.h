//////////////////////////////////////////////////////////////////////////////////////
// BotTalkAction.h - 
//
// Author: Justin Link      
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
// 08/20/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BOTTALKACTION_H_
#define _BOTTALKACTION_H_ 1

#include "fang.h"


FCLASS_NOALIGN_PREFIX class CBotTalkAction
{
public:
	CBotTalkAction();

	f32 m_fStartTime;
	f32 m_fLength;
	u32 m_uData1;	// see BotTalkCsv_ActionType_e
	u32 m_uData2;	// animation index, sound (wave) handle
	u32 m_uData3;	// special flags
	u32 m_uData4;	// User anim slot
private:
	FCLASS_STACKMEM_NOALIGN(CBotTalkAction);
} FCLASS_NOALIGN_SUFFIX;



#endif

