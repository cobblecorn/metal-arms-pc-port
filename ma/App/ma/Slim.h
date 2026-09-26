//////////////////////////////////////////////////////////////////////////////////////
// Slim.h - Slim class for Mettle Arms.
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
// 05/30/02 Link		Created.
// 10/22/02	Elliott		new barter system
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SLIM_H_
#define _SLIM_H_ 1

#include "fang.h"
#include "entity.h"
#include "SSTable.h"
#include "fanim.h"
#include "BarterTypes.h"

class CMeshEntity;
class CFMtx43A;
class CBarterItemInst;

enum SlimAnim_e {
	SLIMANIM_IDLE1 = 0,
	SLIMANIM_IDLE2,
	SLIMANIM_IDLE2A,
	SLIMANIM_IDLE2B,
	SLIMANIM_IDLE2C,
	SLIMANIM_WAVEFLAG,
	SLIMANIM_WAVEHAND,
	SLIMANIM_NODYES,
	SLIMANIM_NODNO,
	SLIMANIM_SHRUG,	
	SLIMANIM_LAUGH,
	SLIMANIM_UNFOLDTABLE,
	SLIMANIM_FOLDTABLE,
	SLIMANIM_PULLOUTSIGN,
	SLIMANIM_FLIPSIGNTO0,// time 0.0f is sign1, 1.0f is sign2
	SLIMANIM_FLIPSIGNTO1,// time 0.0f is sign2, 1.0f is sign1
	SLIMANIM_PUTAWAYSIGN,
	SLIMANIM_THUMBUP,
	SLIMANIM_THUMBDOWN,
	SLIMANIM_LOOKSHADY1,
	SLIMANIM_LOOKSHADY2,

	SLIMANIM_COUNT
};

enum SlimFidget_e {
	SLIMFIDGET_LEGLAUGH	= 0,
	SLIMFIDGET_LEGWAVE,
	SLIMFIDGET_HEADNO,
	SLIMFIDGET_HEADYES,

	SLIMFIDGET_COUNT,
};


FCLASS_NOALIGN_PREFIX class CSlim
{
public:
	CSlim();
	~CSlim();

	BOOL Create(f32 fSpeechDistance);
	void Destroy();

	void Work( BOOL bCanSeePlayer, f32 fDistSqToPlayer );

	void AddToWorld();
	void RemoveFromWorld();
	void Relocate_WS( const CFMtx43A *pMtx_WS );
	void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	//**New Interface functions for BarterSystem
	BOOL ReadyToBarter( void )  { return m_bReadyToBarter; };
	BOOL Available( void )		{ return m_bAvailable; };
	BOOL IsBusy( void )			{ return m_bReportBusy; };
	
	void Welcome( const BarterTypes_TableData_t *pTable, BOOL bFirstTime=FALSE );
	void SetCurrentSlot( u32 nSlotIndex, CBarterItemInst *pNewBII, BOOL bFlipDown );
	
	void Purchase( u32 nUpsaleIndex );
	void FailedPurchase( u32 nUpsaleIndex );
	void RejectedItem( u32 nUpsaleIndex );

	void Exit( u32 nItemsPurchased );
	void NewSale( u32 nUpsaleIndex, const BarterTypes_TableData_t *pNewTable );
	void EventNotify( u32 nEvent, f32 fVal );
	void ResetToPostCreateState( void );
	//**End new BarterSystem interface
	//**

	BOOL ShowFilter( CFVec3A *pPoint );

	void SetCallback( ActionFunction_t *pfcnAction );

	void Set3dAudioRadius( f32 fSpeechRadius) { m_fSpeechRadius = fSpeechRadius;}

	CMeshEntity	*m_pMesh;

private:
	enum SlimFlags_e {
		SLIMFLAG_CREATED		=	1 << 0
	};
	

	////////////////////////
	// Used to Slim's state.
	enum SlimState_e {
		SLIMSTATE_OUT_OF_WORLD = 0,
		SLIMSTATE_ATTRACT_CLOSE,
		SLIMSTATE_ATTRACT_MED,
		SLIMSTATE_ATTRACT_FAR,

		SLIMSTATE_UNFOLDTABLE,			// unfold table and take out sign
		SLIMSTATE_BEGIN_BARTER,
		SLIMSTATE_BARTER,				// regular barter mode, where basically we are just flipping the sign and mixing some idle stuff too	
		SLIMSTATE_BARTER_OVER,			// finishing up the barter thing
		SLIMSTATE_FOLDTABLE,			// put the sign away and fold up the table
		SLIMSTATE_GETSIGN,				// get the sign out
		SLIMSTATE_STOWSIGN,				// stick the sign... somewhere
		SLIMSTATE_BOREDDRUM,			// bored, drum fingers on stomach
		SLIMSTATE_BOREDSIGN,			// bored, going to shrug at the player
		SLIMSTATE_BOREDSHRUG,			// bored, but getting back to work
		SLIMSTATE_BORED_WAITTOBARTER,	// in the process of getting his sign out, when that's done, back to work

		//attract states
		SLIMSTATE_ATTRACTSHRUG,			// these are used as followup animations
		SLIMSTATE_ATTRACTLAUGH,
		SLIMSTATE_ATTRACTBORED,			// do the drum thing
		SLIMSTATE_ATTRACTBORED2,		// put hands back
		SLIMSTATE_ATTRACTLOOKAWAY,
		SLIMSTATE_ATTRACTNOD,
		SLIMSTATE_ATTRACTSHAKE,
		SLIMSTATE_INSULT,				// fun fun

		SLIMSTATE_COUNT
	};

	enum _TAPS {
		_IDLE_TAP0	= 0,
		_IDLE_TAP1,
		_BODY_TAP1,
		_HEAD_TAP1,				//NOTE, the head tap & control are not used exclusively for the head, just most of the time
		_MAIN_TAP1,

		_TAP_COUNT
	};

	enum _CONTROLS {			//this is in order, with main being highest priority
		_MAIN_CONTROL = 0,
		_HEAD_CONTROL,			//NOTE, the head tap & control are not used exclusively for the head, just most of the time
		_BODY_CONTROL,
		_IDLE_CONTROL,

		_CONTROL_COUNT
	};


	SlimState_e m_eCurState;			// what are we doing right now?
	

	FMeshTexLayerHandle_t m_ahTexID[6];		// handles to the sign textures
	
	BarterTypes_TableData_t m_TableData;


	CSSTable			 *m_pTable;
	CFAnimManMtx		 *m_pManMtx;
	CFAnimCombinerConfig *m_pCombinerConfig;
	CFAnimInst			 *m_apAnimInsts[SLIMANIM_COUNT];
	CFAnimCombiner		 *m_pAnimCombiner;

	s32	 m_anTaps[_TAP_COUNT];
	BOOL m_abPaused[_TAP_COUNT];
	BOOL m_abClampAtEnd[_TAP_COUNT];
	f32	 m_afAnimTimes[_TAP_COUNT];

	CFAnimInst *m_apCurrentAnims[_TAP_COUNT];

	s32  m_anControls[_CONTROL_COUNT];
	BOOL m_abAnimateControl[_CONTROL_COUNT];
	f32  m_afControlAnimTime[_CONTROL_COUNT];
	f32  m_afOOControlAnimTime[_CONTROL_COUNT];
	f32  m_afControlStart[_CONTROL_COUNT];
	f32  m_afControlEnd[_CONTROL_COUNT];
	f32	 m_afControlResetTime[_CONTROL_COUNT];
	f32	 m_afOOControlResetTime[_CONTROL_COUNT];
	BOOL m_abControlReset[_CONTROL_COUNT];

	f32	 m_fIdleAnimSpeedAdj;				// speed up / slow down idle for variety, 1.0 is baseline
	f32  m_fFidgetTimeAdj;					// play this slower or faster? 1.0 is baseline
	f32  m_fTimer;							// general timer, used in different ways in different states
	f32  m_fTimeTillSelectAnim;				// time left to select new anim
	f32	 m_fPatienceTimer;					// slim gets impatient, then does the finger drumming thing
	u32	 m_nPatienceCounter;				// if he gets too impatient...

	BOOL m_bReadyToBarter;					// ready?
	BOOL m_bAvailable;						// ready to get ready to barter
	BOOL m_bReportBusy;						// report busy to the barter mgr
	BOOL m_bDelayedExit;					// used to postpone the exit until the bored animations are complete
	
	u32	 m_nLastPrice;						// the table's last price.  Used to make sure the table hasn't changed while we're flipping signs around
	BOOL m_bSignSide;						// alternates between sign 0 & 1
	s32	 m_nMood;							// stores how many items glitch purchased on exit, so later, we can react correctly
	BOOL m_bDrawInsultFilter;				// draw the insult filter?

	u32  m_uFlags;							// store whether or not we've been created
	f32	 m_fSpeechRadius;
	static u32	m_uFilterBone;						// the insult bone

	static FSndFx_FxHandle_t m_hLaughSound;

	static SlimAnim_e	m_anFidgetAnims[SLIMFIDGET_COUNT];				// animations to be used for fidgets
	static f32			m_afFidgetTimeAdj[SLIMFIDGET_COUNT];			// time adjustment for fidgets, primarily to slow them down
	static f32			m_afFidgetMaxBlend[SLIMFIDGET_COUNT];			// max amount to blend into idle
	static cchar	  **m_apszFidgetBones[SLIMFIDGET_COUNT];			// the bones involved

	


	BOOL IsCreated() { return((m_uFlags & SLIMFLAG_CREATED) != 0); };

	void _ConfigIdleState();

	void _StartFidget( u32 nFidget );
	void _SelectIdleAnimation( void );
	void _Think( void );
	void _ResetPatience( void );

	void _FlipSign( void );
	void _RefreshSign( void );	
	
	void _DoGoodbye( void );

	BOOL _InitInsult( void );
	void _InsultPlayer( void );
	


	//ANIMATION CONTROL FNS
	BOOL _LoadAndConfigAnimations();
	void _FreeAnimationVars();
	BOOL _IsAnimAtEnd( f32 fUnitThresholdToEnd=0.99f, s32 nTapIndex=-1 );
	BOOL _AnimComplete( s32 nTapIndex );										//checks the actual animation time, not the anim unit time
	void _AnimWork();
	void _AddBlendedAnimation( SlimAnim_e nNewAnimation,
							   u32 nTapId,
							   u32 nControlID,
							   BOOL bClampMode,
							   BOOL bPause,
							   f32 fUnitTime,
							   f32 fBlendTime,
							   f32 fTargetControlValue,
							   f32 fResetTime,						//time it takes to put the control back to its starting value (0 to leave it)
							   f32 fAnimTime,						//time to run the animation.  Ignored if bClampMode == TRUE
							   cchar **apszBonesToDrive/*=NULL*/ );
	void _SetupControlAnim( u32 nControlID, f32 fBlendTime, f32 fTargetControlValue, f32 fResetTime );
	void _CheckControlReset( u32 nControlID, u32 nTapID );

	
	FCLASS_STACKMEM_NOALIGN(CSlim);
} FCLASS_NOALIGN_SUFFIX;

#endif
