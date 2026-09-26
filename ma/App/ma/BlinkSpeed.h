//////////////////////////////////////////////////////////////////////////////////////
// BlinkSpeed.h - 

//      - class CLightningBoltGroup
//		- class CLightningBolt
//		- class CBlinkSpeed
//
// Author: mrs. Justin Link 
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
// 07/22/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BLINKSPEED_H_
#define _BLINKSPEED_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fdraw.h"


class CBot;
class CFXfm;
struct FkDOP_Geo_t;
class CLightningBolt;


static const u32 _LBBoneListSize = 10;
static const u32 _LBG_MaxBolts = 50;
static const u32 _LB_MaxBranches = 10;


FCLASS_NOALIGN_PREFIX class CLightningBoltGroup
{
public:
	static BOOL InitSystem();		// fnew Pool, load/create texture, call _Clear() on each one.
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static void Work();		// Go through all of the groups, call _Work() if they're active.
	static void FDraw();		// Go through all of the groups, call _FDraw() if they're active.

	enum LightningType_e
	{
		LTYPE_NORMAL = 0,

		LTYPE_COUNT
	};

	static CLightningBoltGroup *GetLightning();
	BOOL ReturnLightning();

	BOOL Init(u32 uNumBolts, CBot *pBot, u8 uNumBoneNames, cchar **papszBoneNames);
	void Activate();
	void SetUnitEffectEnd(f32 fUnitEffectEnd);		// This tells the system how far into the 'end effect' that the bolts should be.
	void Deactivate();

private:
	enum Flags_e
	{
		FLAG_ACQUIRED		=	1 << 0,				// Someone has Acquire()d this group.
		FLAG_ACTIVE			=	1 << 1,				// Someone has called Activate() on this group.
		FLAG_NONE			=	0
	};

	CLightningBoltGroup();		// Call _Clear();
	~CLightningBoltGroup();

	void _Clear();
	void _Work();			// Go through each bolt, if finished, reinit, then call Work() on all.
	void _FDraw(const CFXfm *pCamXfm);			// Call FDraw() on each bolt.

	//
	//  Static Data
	//
	static const u32 s_uPoolSize;
	static CLightningBoltGroup *s_paLBPool;		// fnew in InitSystem();
	static CFTexInst s_oTexInst;


	FCLASS_NOALIGN_PREFIX class CLightningBolt
	{
	public:
		CLightningBolt();
		~CLightningBolt();

		void Init(s32 nBoneIdx, f32 fLifeTime);

		void Work();		// Update m_fAlpha.
		void FDraw(const CFXfm *pCamXfm, CFMtx43A *pmtxBS2WS);		// 

		f32 m_fCurAlpha;
		s32 m_nBoneIdx;
	private:

		FCLASS_NOALIGN_PREFIX class CLBBranch
		{
		public:
			CLBBranch();
			~CLBBranch();

			void Init(f32 fTheta, f32 fLength);
			void AddToVtxList(FDrawVtx_t *pavtxArray, u32 &uCurVtx, const CFVec3A &vecParentPos, const CFVec3A &vecRight_WS, const CFVec3A &vecUp_WS, f32 fAlpha);

			CLBBranch *m_apChildren[2];
			CFVec2 m_vecUnitOfsFromParent;
			f32 m_fDistFromParent;

		private:
			// Set pointers to NULL in _Clear();
			void _Clear();

			FCLASS_STACKMEM_NOALIGN(CLBBranch);
		} FCLASS_NOALIGN_SUFFIX;

		// The pBuiltBoltHead parameter is actually a return value.  It tells you which branch was the head of the bolt
		//   that it built.  It is fine to pass in NULL for this parameter.
		// For some reason, I couldn't get it to use a CLBranch * as a return value, don't ask me why.
		void _BuildBolt(CLBBranch **pBuiltBoltHead, u32 &uStartBranchIdx, u32 uDepth, f32 fThetaIn, f32 fLengthIn);

		CLBBranch m_aBranches[_LB_MaxBranches];
		f32 m_fDistFromBoneOrigin;	// Set in Init().
		f32 m_fFadeRate;			// Set in Init().
		f32 m_fOOFadeRate;			// Set in Init().

		FCLASS_STACKMEM_NOALIGN(CLightningBolt);
	} FCLASS_NOALIGN_SUFFIX;

	void _InitBolt(CLightningBolt *pBolt);


	s16 m_anBoneIdxList[_LBBoneListSize];		// Set all to -1 in _Clear();
	u8 m_uBoneListSize;							// Set to 0 in _Clear().
	u8 m_uFlags;
	f32 m_fUnitEffectEnd;						// Set to 0.0f in Activate().

	CBot *m_pBot;
	u32 m_uNumBolts;
	CLightningBolt m_aoLB[_LBG_MaxBolts];

	FCLASS_STACKMEM_NOALIGN(CLightningBoltGroup);
} FCLASS_NOALIGN_SUFFIX;





FCLASS_ALIGN_PREFIX class CBlinkSpeed
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static CBlinkSpeed *GetSpeed();
	BOOL ReturnSpeed();

	static void Work();
	static void FDraw();

	void Activate();
	void SetBotBlink(CBot *pBot)					{ m_pBot = pBot; };
	void SetUnitEffectEnd(f32 fUnitEffectEnd);		// This tells the system how far into the 'end effect' that the bolts should be.
	void Deactivate();

private:

	enum _DopBones_e
	{
		DOPBONE_RLEGLOWER	=	0,
		DOPBONE_LLEGLOWER,
		DOPBONE_RHEEL,
		DOPBONE_LHEEL,

		DOPBONE_COUNT
	};

	static BOOL m_bSystemInitted;
	static CBlinkSpeed *m_apSpeedPool;
	BOOL m_bAllocated;

	static cchar *s_apszBoneNames[DOPBONE_COUNT];

	CBlinkSpeed();
	~CBlinkSpeed();

	CBot *m_pBot;
	CLightningBoltGroup *m_pBoltGroup;

	//////////////////////////////////////////////////////////////////////
	//
	static CFVec2 m_vecSTVel, m_vecSTPos;
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

	FCLASS_STACKMEM_ALIGN(CBlinkSpeed);
} FCLASS_ALIGN_SUFFIX;



#endif

