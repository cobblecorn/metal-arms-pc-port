//////////////////////////////////////////////////////////////////////////////////////
// BotTalkInst.h - 
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
#ifndef _BOTTALKINST_H_
#define _BOTTALKINST_H_ 1

#include "fang.h"
#include "TalkSystem2.h"
#include "botanim.h"



class CBot;
class CFAnimInst;
class CFAudioEmitter;
class CBotTalkData;

// TODO: Isworking should moved to be private.
enum
{
	BOTTALKINSTFLAG_HIGHPRIORITY			=	0x00000001,
	BOTTALKINSTFLAG_GIVECHIP				=	0x00000002,
	BOTTALKINSTFLAG_TRIGGEREVENTSTART		=	0x00000004,
	BOTTALKINSTFLAG_TRIGGEREVENTEND			=	0x00000008,
	BOTTALKINSTFLAG_ISWORKING				=	0x00000010,
	BOTTALKINSTFLAG_HASREACHEDEND			=	0x00000020,
	BOTTALKINSTFLAG_3DSOUND_PLAYING			=	0x00000040,		// the audio that is being played is 3d
	BOTTALKINSTFLAG_FORCE_2D_AUDIO			=	0x00000080,		// an over ride that will force any audio to be played 2d, regardless of what the data says
	BOTTALKINSTFLAG_UPDATE_3D_POS			=  	0x00000100,		// next work function, update the 3d audio position
	BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING =	0x00000200,	// The audio for this instance is NON-DUCKABLE
	BOTTALKINSTFLAG_AUDIO_DAMAGED			=	0x00000400,	// this bot's voice is damaged
	BOTTALKINSTFLAG_AUDIO_FADETOSTOP		=	0x00000800,	// too damaged to continue, just fade out
	BOTTALKINSTFLAG_AUDIO_STUTTER			=	0x00001000,	// audio damaged.  Stutter
	BOTTALKINSTFLAG_WAIT_FOR_STREAM_LOAD	=	0x00002000,	// This bottalk has streams, and should wait for load
};



FCLASS_NOALIGN_PREFIX class CBotTalkInst
{
public:
	CBotTalkInst();
	~CBotTalkInst();

	BOOL Init(cchar *pszSourceFileName);
	BOOL Init(CBotTalkData *pTalkData);
	void Destroy();

	// All calls to this method should come through CTalkSystem2.
	BOOL Start(CBot *pBotToRun, BotTalkInstDoneCallback_t* pfcnTalkDoneCallback = NULL );
	void Work();
	BOOL End(BOOL bCanStick = FALSE);	//return TRUE if BTI actually ended, bCanStick means that it is o.k. for the BTI to decide not to end if it's flags say it should stick at the end of play

	void Force2dAudio()						{ m_uFlags |= BOTTALKINSTFLAG_FORCE_2D_AUDIO; }
	void SetAudioRadius( f32 fNewRadius )	{ m_fRadius = fNewRadius; }
	void DuckAudioWhenPlaying( BOOL bDuck )
	{
		if(bDuck)
		{
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING); 
		}
		else
		{
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING); 
		}
	}


	BOOL IsHighPriority()					{ return(m_uFlags & BOTTALKINSTFLAG_HIGHPRIORITY); }
	BOOL IsGiveChip()						{ return(m_uFlags & BOTTALKINSTFLAG_GIVECHIP); }
	BOOL IsTriggerEventStart()				{ return(m_uFlags & BOTTALKINSTFLAG_TRIGGEREVENTSTART); }
	BOOL IsTriggerEventEnd()				{ return(m_uFlags & BOTTALKINSTFLAG_TRIGGEREVENTEND); }
	BOOL IsWorking()						{ return(m_uFlags & BOTTALKINSTFLAG_ISWORKING); }
	BOOL IsWaitingForStreamLoad()			{ return(m_uFlags & BOTTALKINSTFLAG_WAIT_FOR_STREAM_LOAD); }
	void SetWaitingForStreamLoad(BOOL bWait)
	{
		if(bWait)
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_WAIT_FOR_STREAM_LOAD); 
		else
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_WAIT_FOR_STREAM_LOAD);
	}

	f32  GetCurrentUnitTime( void ) const;

	void SetHighPriority(BOOL bHighPriority)
	{
		if(bHighPriority)
		{
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_HIGHPRIORITY); 
		}
		else
		{
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_HIGHPRIORITY);
		}
	}

	void SetTalkReason(TalkActivity2_e eTalkActivity)				{ m_eTalkActivity = eTalkActivity;}
	TalkActivity2_e GetTalkReason()									{ return m_eTalkActivity;}

	void SetTalkDoneCB(BotTalkInstDoneCallback_t *pfcnTalkDoneCallback)	{ m_pfcnTalkDoneCallback = pfcnTalkDoneCallback;}
	BotTalkInstDoneCallback_t * GetTalkDoneCB(void)						{ return m_pfcnTalkDoneCallback;}

	void SetFlags(u32 uFlags)										{ m_uFlags = uFlags;}
	u32 GetFlags(void)												{ return m_uFlags;}
	FINLINE CBotTalkData* GetTalkData(void)							{ return m_pTalkData;}

	CFAudioEmitter *m_pCurSound;

	FLink_t	m_TS2Next;					   //For management by TalkSystem2

private:
	void _Clear();
	void _GivePlayerChip();
	void _HandleAudioDamageEffects( void );

	void _SetIsWorking(BOOL bIsWorking)
	{
		if(bIsWorking)
		{
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_ISWORKING); 
		}
		else
		{
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_ISWORKING);
		}
	}

	//
	CBot *m_pBotTarget;
	u32 m_uFlags;
	f32 m_fRadius;
	TalkActivity2_e m_eTalkActivity;
	BotTalkInstDoneCallback_t *m_pfcnTalkDoneCallback;

	
	
	//////////////////////////////////////////////////////////////////////
	// These arrays will contain the start time and end time for an
	//   animation in each user slot.  They will have the value -1.0f if
	//   there is no currently active animation there.

	f32 m_afStartTime[CBotAnimStackDef::ANIM_USER_COUNT];
	f32 m_afEndTime[CBotAnimStackDef::ANIM_USER_COUNT];
	s32 m_anAnimInstIdx[CBotAnimStackDef::ANIM_USER_COUNT];
	//
	//////////////////////////////////////////////////////////////////////

	f32 m_fCurTimePos;
	f32	m_fNextDamageEffectTime;				// next time to play a damage effect (stutter, fade out, etc)
	f32 m_fUnitDamageEffect;					// used to measure current damage effect value (for instance, what our current pitch is)

	CBotTalkData *m_pTalkData;
	u32 m_uNextActionIdx;

	CFAnimInst *m_paAnimInst;


	FCLASS_STACKMEM_NOALIGN(CBotTalkInst);
} FCLASS_NOALIGN_SUFFIX;



















#define NUM_BT_CHANNELS 4
FCLASS_NOALIGN_PREFIX class CTalkInst
{
public:
	CTalkInst();
	~CTalkInst();

	BOOL Init(cchar *pszSourceFileName);
	BOOL Init(CBotTalkData *pTalkData);
	void Destroy();

	// All calls to this method should come from CTalkSystem2.
	BOOL Start(	CFAnimCombiner* pCombiner,
				const s32 *panTapIds,
				const s32 *panCtrlIds,
				u32 uUserData = 0,
				TalkInstDoneCallback_t* pfcnTalkDoneCallback = NULL );
	void Work();
	BOOL End(BOOL bCanStick = FALSE);	//return TRUE if BTI actually ended, bCanStick means that it is o.k. for the BTI to decide not to end if it's flags say it should stick at the end of play

	BOOL IsAtEnd(void);

	BOOL IsHighPriority() { return(m_uFlags & BOTTALKINSTFLAG_HIGHPRIORITY); }
	BOOL IsTriggerEventStart() { return(m_uFlags & BOTTALKINSTFLAG_TRIGGEREVENTSTART); }
	BOOL IsTriggerEventEnd() { return(m_uFlags & BOTTALKINSTFLAG_TRIGGEREVENTEND); }
	BOOL IsWorking() { return(m_uFlags & BOTTALKINSTFLAG_ISWORKING); }
	BOOL IsWaitingForStreamLoad()			{ return(m_uFlags & BOTTALKINSTFLAG_WAIT_FOR_STREAM_LOAD); }
	void SetWaitingForStreamLoad(BOOL bWait)
	{
		if(bWait)
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_WAIT_FOR_STREAM_LOAD); 
		else
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_WAIT_FOR_STREAM_LOAD);
	}


	void SetHighPriority(BOOL bHighPriority)
	{
		if(bHighPriority)
		{
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_HIGHPRIORITY); 
		}
		else
		{
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_HIGHPRIORITY);
		}
	}

	void SetTalkReason(TalkActivity2_e eTalkActivity)				{ m_eTalkActivity = eTalkActivity;}
	TalkActivity2_e GetTalkReason()									{ return m_eTalkActivity;}

	void SetDoneCB(TalkInstDoneCallback_t *pfcnDoneCallback)		{ m_pfcnDoneCallback = pfcnDoneCallback;}
	TalkInstDoneCallback_t * GetTalkDoneCB(void)					{ return m_pfcnDoneCallback;}

	void SetFlags(u32 uFlags)										{ m_uFlags = uFlags;}
	u32 GetFlags(void)												{ return m_uFlags;}
	FINLINE CBotTalkData* GetTalkData(void)							{ return m_pTalkData;}

	void DuckAudioWhenPlaying( BOOL bDuck )
	{
		if(bDuck)
		{
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING); 
		}
		else
		{
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING); 
		}
	}
	void Set3DPos( const CFVec3A& Pos_WS, f32 fSoundRadius )		{ m_uFlags |= BOTTALKINSTFLAG_UPDATE_3D_POS; m_WherePos_WS = Pos_WS.v3 ; m_fRadius = fSoundRadius; }
	CFAudioEmitter *m_pCurSound;

	f32 GetTimeUnit(void);

	FLink_t	m_TS2Next;					   //For management by TalkSystem2

private:
	void _Clear();

	void _SetIsWorking(BOOL bIsWorking)
	{
		if(bIsWorking)
		{
			FMATH_SETBITMASK(m_uFlags, BOTTALKINSTFLAG_ISWORKING); 
		}
		else
		{
			FMATH_CLEARBITMASK(m_uFlags, BOTTALKINSTFLAG_ISWORKING);
		}
	}

	
	CFVec3 m_WherePos_WS;
	f32	m_fRadius;
	//
	s32 m_panTapIds[NUM_BT_CHANNELS];
	s32 m_panCtrlIds[NUM_BT_CHANNELS];
	u32 m_uUserData;
	u32 m_uFlags;
	TalkActivity2_e m_eTalkActivity;
	TalkInstDoneCallback_t *m_pfcnDoneCallback;

	CFAnimCombiner* m_pAnimCombiner;
	
	//////////////////////////////////////////////////////////////////////
	// These arrays will contain the start time and end time for an
	//   animation in each user slot.  They will have the value -1.0f if
	//   there is no currently active animation there.

	f32 m_afStartTime[NUM_BT_CHANNELS];
	f32 m_afEndTime[NUM_BT_CHANNELS];
	s32 m_anAnimInstIdx[NUM_BT_CHANNELS];
	//
	//////////////////////////////////////////////////////////////////////
	f32 m_fCurTimePos;
	u32 m_uNextActionIdx;
	CBotTalkData *m_pTalkData;
	CFAnimInst *m_paAnimInst;

	

	FCLASS_STACKMEM_NOALIGN(CTalkInst);
} FCLASS_NOALIGN_SUFFIX;

#endif
