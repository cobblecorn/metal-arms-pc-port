//////////////////////////////////////////////////////////////////////////////////////
// BlinkGlow.h - BlinkGlow class for Metal Arms.
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
// 07/31/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BLINKGLOW_H_
#define _BLINKGLOW_H_ 1

#include "fang.h"
#include "fdraw.h"



class CBotGlitch;
struct FkDOP_Geo_t;



FCLASS_NOALIGN_PREFIX class CBlinkGlow
{
public:
	enum DopBone_e
	{
		DOPBONE_RLEGLOWER	=	0,
		DOPBONE_LLEGLOWER,
/*		DOPBONE_GROIN,
		DOPBONE_BEDROLL,
		DOPBONE_BACKPACK,
		DOPBONE_TORSO,
		DOPBONE_RARMUPPER,
		DOPBONE_LARMUPPER,
		DOPBONE_RARMLOWER,
		DOPBONE_LARMLOWER,
		DOPBONE_LARMELBOW,
		DOPBONE_LHAND,
		DOPBONE_RSHOULDER,
		DOPBONE_LSHOULDER,
		DOPBONE_HEAD,*/
		DOPBONE_RHEEL,
		DOPBONE_LHEEL,

		DOPBONE_COUNT
	};

	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static CBlinkGlow *GetGlow();
	BOOL ReturnGlow();

	static void Work();
	static void FDraw();

	void Activate();			// Has default values for color.
	void SetColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha1, f32 fAlpha2);
	void SetBotBlink(CBotGlitch *pBot) { m_pBot = pBot; };
	void SetDrawEnabled(BOOL bDrawEnabled) { m_bDrawEnabled = bDrawEnabled; }
	void EnableDOPBone(DopBone_e eDOPBone);
	void DisableDOPBone(DopBone_e eDOPBone);
	void SetUnitEffectEnd(f32 fUnitEffectEnd);		// This tells the system how far into the 'end effect' that the glow should be.
	void Deactivate();

private:
	CBlinkGlow();
	~CBlinkGlow();

	void _Work();
	void _FDraw();
	void _AddToVtxList(FDrawVtx_t *pVtxArray, u32 &uCurVtx, u32 uDOPIdx, u32 uFaceIdx, u32 uVrtIdx, CFMtx43A *pBS2WS, CFVec3A &vecBlinkFront, f32 fGlobalAlpha, CFColorRGB &rgbBase);

	//////////////////////////////////////////////////////////////////////
	//
	static BOOL m_bSystemInitted;
	static CBlinkGlow *m_apGlowPool;
	static u32 m_nNextInPool;

	static const FkDOP_Geo_t *s_apGeo[DOPBONE_COUNT];
	static cchar *s_apszBoneNames[DOPBONE_COUNT];
	static const f32 s_afScale[DOPBONE_COUNT];
	//
	//////////////////////////////////////////////////////////////////////

	f32 m_fUnitEffectEnd;
	BOOL m_bAllocated;

	CBotGlitch *m_pBot;
	BOOL m_bDrawEnabled;			// Set to TRUE in Activate().

	//////////////////////////////////////////////////////////////////////
	//
	CFColorRGB m_rgbColor;			// Set to default in Activate().
	f32 m_afAlpha[2];
	//
	//////////////////////////////////////////////////////////////////////

	u32 m_uDOPFlags;				// These bits are set according to what DOPs should be glowing.

	//////////////////////////////////////////////////////////////////////
	//
	enum Flags_e
	{
		FLAG_ACTIVATED		=	1 << 0,

		FLAG_NONE			=	0
	};
	u32 m_uFlags;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// State.
	enum State_e
	{
		STATE_ACTIVATING	=	0,
		STATE_ACTIVE,

		STATE_COUNT
	};
	State_e m_eState;
	f32 m_fStateTimer;
	//
	//////////////////////////////////////////////////////////////////////

	FCLASS_STACKMEM_NOALIGN(CBlinkGlow);
} FCLASS_NOALIGN_SUFFIX;


#endif

