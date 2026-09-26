//////////////////////////////////////////////////////////////////////////////////////
// BotTalkData.h - 
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
#ifndef _BOTTALKDATA_H_
#define _BOTTALKDATA_H_ 1

#include "fang.h"
#include "fdata.h"
#include "BotTalkCsvFlags.h"

class CBotTalkAction;
struct FAnim_t;

FCLASS_NOALIGN_PREFIX class CBotTalkData
{
public:
	CBotTalkData();

	BOOL LoadFromFile(cchar *pszFileName);

	BOOL IsInitted() { return(BOOL)(m_uFlags & BOT_TALK_CSV_FLAG_IS_INITTED); }
	u32 DrivesBones() { FASSERT(IsInitted()); return ( m_uFlags & BOT_TALK_CSV_FLAG_DRIVES_BONE_MASK ); }
	BOOL DrivesLowerBody() {FASSERT(IsInitted()); return (m_uFlags & BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY)!=0;}
	BOOL HasTalkLoop() { FASSERT(IsInitted()); return(BOOL)(m_uFlags & BOT_TALK_CSV_FLAG_HAS_TALK_LOOP); }
	BOOL NoBlendOut() { FASSERT(IsInitted()); return(BOOL)(m_uFlags & BOT_TALK_CSV_FLAG_NO_BLEND_OUT); }
	BOOL StickAtEnd() { FASSERT(IsInitted()); return(BOOL)(m_uFlags & BOT_TALK_CSV_FLAG_STICK_AT_END); }
	BOOL Use2DSound() { FASSERT(IsInitted()); return(BOOL)(m_uFlags & BOT_TALK_CSV_FLAG_2DSOUNDS); }

	cchar *m_pszSourceFileName;
	f32 m_fTotalTime;

	u32 m_uAnimCnt;
	FAnim_t **m_papAnims;

	u32 m_uActionCnt;
	CBotTalkAction *m_paActions;

private:
	void Clear();

	u32 m_uFlags;	// bitmask, see BotTalkCsvFlags_e

	FLink_t* m_TS2Next;					   //For management by TalkSystem2

	FCLASS_STACKMEM_NOALIGN(CBotTalkData);
} FCLASS_NOALIGN_SUFFIX;

#endif
