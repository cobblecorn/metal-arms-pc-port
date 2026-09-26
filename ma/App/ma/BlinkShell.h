//////////////////////////////////////////////////////////////////////////////////////
// BlinkShell.h - 
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
// 07/12/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BLINKSHELL_H_
#define _BLINKSHELL_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fdraw.h"


class CBotGlitch;


FCLASS_ALIGN_PREFIX class CBlinkShell
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static CBlinkShell *GetShell();
	BOOL ReturnShell();

	static void Work();
	static void FDraw();

	void Activate();
	void SetBotBlink(CBotGlitch *pBot) { m_pBot = pBot; };
//	void BotGotHit(CDamageInfo *pDamageInfo);
	void SetUnitEffectEnd(f32 fUnitEffectEnd);		// This tells the system how far into the 'end effect' that the bolts should be.
	void Deactivate();

private:
	CBlinkShell();
	~CBlinkShell();

	CBotGlitch *m_pBot;

	f32 m_fHotSpotFactor;
	CFVec3A m_vecHitPos_WS;
//	CFVec3A m_vecHitPos_MS;
	CFVec3A m_vecHitPos_BS;
//	s32 m_nHitDOPIdx;
	s32 m_nHitBoneIdx;
	s32 m_nHitFaceIdx;

	f32 m_fUnitEffectEnd;
	BOOL m_bAllocated;

	static BOOL m_bSystemInitted;
	static CBlinkShell *m_apShellPool;
	static CFTexInst s_oTexInst;

	//////////////////////////////////////////////////////////////////////
	//
	enum State_e
	{
		STATE_INACTIVE	=	0,
		STATE_ACTIVATING,
		STATE_ACTIVE,
		STATE_DEACTIVATING,

		STATE_COUNT
	};

	f32 m_fStateTimer;
	f32 m_fDOPGlowPos;			// All DOPs whose indices are equal to less than this value will be given a glow
								//   based upon how close they are to this value.
	BOOL m_bAlreadyGlowed;		// This is to ensure that our glow only passes through Glitch once.

	State_e m_eState;
	//
	//////////////////////////////////////////////////////////////////////

	void _Work();
	void _FDraw();

	enum Flags_e
	{
		FLAG_ACTIVATED		=	1 << 0,

		FLAG_NONE			=	0
	};
	u32 m_uFlags;

	void _AddToVtxList(FDrawVtx_t *pVtxArray, u32 &uCurVtx, u32 uDOPIdx, u32 uFaceIdx, u32 uVrtIdx, CFMtx43A *pBS2WS, CFVec3A &vecBlinkFront, f32 fGlobalAlpha, CFColorRGB &rgbBase);

	FCLASS_STACKMEM_ALIGN(CBlinkShell);
} FCLASS_ALIGN_SUFFIX;


#endif
