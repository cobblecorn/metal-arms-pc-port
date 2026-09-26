//////////////////////////////////////////////////////////////////////////////////////
// TalkSystem2.h - New Talking System for Mettle Arms.
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
// 05/28/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _TALKSYSTEM2_H_
#define _TALKSYSTEM2_H_ 1

#include "fang.h"
#include "fdata.h"

class CEntity;
class CBot;
class CFMeshInst;
struct FAnim_t;
class CFAnimInst;
class CBotTalkData;
class CBotTalkInst;
class CTalkInst;
class CFAnimCombiner;

typedef void BotTalkInstDoneCallback_t(CBotTalkInst *pTalkInst, CBot *pBot);
typedef void TalkInstDoneCallback_t(CTalkInst *pTalkInst, u32 uUser);



enum TalkActivity2_e
{
	TALKACTIVITY2_NONE,
	TALKACTIVITY2_AMBIENT,
	TALKACTIVITY2_NONAMBIENT,  // pauses the game and freezes human player controls until the talk is over
};

static const u32 CTalkSystem2_uMaxSimulBotTalkInst = 70;
static const u32 CTalkSystem2_uMaxSimulTalkInst = 10;

FCLASS_NOALIGN_PREFIX class CTalkSystem2
{
public:
	static BOOL InitSystem();
	static BOOL InitLevel();
	static void UninitLevel();
	static void UninitSystem();

	static void Work();

	//  Playing a BTA through a CBot
	static BOOL SubmitTalkRequest(CBotTalkInst *pTalkInst, CBot *pBot);
	static BOOL TerminateActiveTalk(CBotTalkInst *pTalkInst);


	// Playing BTA on a combiner 
	static BOOL SubmitTalkRequest(	CTalkInst *pTalkInst,
									CFAnimCombiner* pCombiner,
									const s32 *panTapIds,
									const s32 *panCtrlIds,
									u32 uUserData,
									TalkInstDoneCallback_t* pfcnTalkDoneCallback = NULL );
	static BOOL TerminateActiveTalk(CTalkInst *pTalkInst);


	static CBotTalkData *GetTalkDataByFileName(cchar *pszFileName);
	static BOOL BTIPool_Init(cchar *pszBTAFileName, u32 uNumToAlloc);
	static CBotTalkInst *BTIPool_Get(cchar *pszBTAFileName);
	static void BTIPool_Recycle(CBotTalkInst* pInst);

	static BOOL ControlsAreCaptive();

	static CFMeshInst* GetChipMeshInst();

	static s32 m_nTalkEvent;

private:
	CTalkSystem2();
	~CTalkSystem2();

	static CFMeshInst *m_pMeshChip;

	static u32 s_uNumTalkDatas;
	static u32 s_uSystemTalkDataCnt;

	static FLinkRoot_t m_TalkResourcePool;
	static CBotTalkInst *m_apBotTalkInst[CTalkSystem2_uMaxSimulBotTalkInst];
	static CTalkInst *m_apTalkInst[CTalkSystem2_uMaxSimulTalkInst];
	static u32 s_uNonAmbientCount;

	FCLASS_STACKMEM_NOALIGN(CTalkSystem2);

} FCLASS_ALIGN_SUFFIX;

#endif
