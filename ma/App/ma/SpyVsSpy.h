//////////////////////////////////////////////////////////////////////////////////////
// SpyVsSpy.h - 
//
// Author: Nathan Miller
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
// 03/20/03 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SPYVSSPY_H_
#define _SPYVSSPY_H_ 1

#include "fang.h"
#include "level.h"

class CMeshEntity;
class CBotGlitch;
class CSearchStage;
class CHumanControl;

typedef struct {
	// DDR related values
	f32 fDDRCommandTimeMin;
	f32 fDDRCommandTimeMax;
	
	f32 fDDRMoveTimeMin;
	f32 fDDRMoveTimeMax;

	// Used to test Glitch's alignment
	f32 fDDRAlignFailTime;
	f32 fDDRPositionFailTime;
	f32 fDDRLookFailTime;

	f32 fDDRMoveDistance;
	f32 fDDRLagDistance;
	f32 fDDRBumpUpDistance;
	u32 uDDRMoveTimeMin;
} SpyVsSpyConfigValues_t;


//
//
//
// CSpyVsSpyStage
//
//
//
FCLASS_ALIGN_PREFIX class CSpyVsSpyStage {
//----------------------------------------------------------------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CSpyVsSpyStage()			{ _SetFinished( TRUE ); }
	virtual ~CSpyVsSpyStage()	{ }

	FINLINE BOOL IsFinished( void ) { return m_bFinished; }

	virtual BOOL Load( LevelEvent_e eEvent ) { return FALSE; }
	virtual void Unload( void )		{ }
	virtual void Restore( void )	{ } 
	virtual void Work( void )		{ }
	virtual void SwitchTo( void )   { }
	virtual void Cleanup( void )	{ }

protected:
	FINLINE void _SetFinished( BOOL bFin ) { m_bFinished = bFin; }

protected:
	BOOL m_bFinished;

	FCLASS_STACKMEM_ALIGN( CSpyVsSpyStage );
} FCLASS_ALIGN_SUFFIX;


//
//
//
// CSpyVsSpy
//
//
//
FCLASS_ALIGN_PREFIX class CSpyVsSpy {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL LoadLevel( LevelEvent_e eEvent );
	static void UnloadLevel( void );
	static void Work( void );

	static void SetupGlitchPointer( CBotGlitch *pGlitch );

	static void CheckpointRestore( s32 nCheckpoint );

	// Utility functions
	static void StreamStart( cchar *pszName, BOOL bBlock = FALSE, BOOL bCanSkip = FALSE );

	static f32 GetIdleTime( f32 fUnitTime );
	static f32 GetCommandTime( f32 fUnitTime );
	static f32 GetMoveTime( f32 fUnitTime );

	static void TurnOffHUD( BOOL bOff, BOOL bNoReticle = FALSE, BOOL bNoWeaponSel = FALSE );
	static void AttackDisable( BOOL bOff );
	static void CoopSyncStage( cchar *pszPrimary, cchar *pszSecondary, BOOL bRegroup );
#if FANG_WINGC
	static BOOL IsFactoryActive() { return m_bLevelInitted && m_pGame; }
	static BOOL PortTestStage( u32 nStage );
	static BOOL PortTestDance( u32 nTest );
	static BOOL PortTestEscort(u32 test);
	static BOOL PortTestInspection(u32 test);
	static BOOL CoopIndividualViews();
	static BOOL CoopPackingWaiting();
	static void ConstrainPackingControls(CEntity *body, CHumanControl *controls);
#endif

	static CEntity *FindEntity( cchar *pszName, const u64 &uTypeBits = 0, BOOL bRequired = TRUE );
	static void TakeControlFromPlayer( BOOL bTake, BOOL bCanSkip = FALSE );
	static void CameraInterp( const f32 &fUnitTime,
						const f32 &fStartFOV, const CFVec3A &StartPos, const CFQuatA &StartQuat,
						const f32 &fEndFOV, const CFVec3A &EndPos, const CFQuatA &EndQuat,
						f32 *fCurrFOV, CFVec3A *CurrPos, CFQuatA *CurrQuat );

	static void MakeBotDumb( CBot *pBot, f32 fSpeedPercent );
	static void MakeBotSmart( CBot *pBot );
	static void PlayBotTalk( CBot *pBot, u32 uType, f32 *fTime = NULL );

	static void ListenerOrientationCallback( u32 uPlayerIndex );

	static FINLINE CBotGlitch *GetGlitch( void ) { FASSERT( m_pCommonData ); return m_pCommonData->m_pGlitch; }

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	CSpyVsSpy() { _ClearDataMembers(); }
	virtual ~CSpyVsSpy() { }

	BOOL _Load( LevelEvent_e eEvent );
	void _Unload( void );
	void _Work( void );

	void _ClearDataMembers( void );
	void _CheckpointRestore( s32 nCheckpoint );
	static BOOL _LoadLevelCSV( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Public definitions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	enum {
		BTR_BACK = 0,
		BTR_CBACK,
		BTR_FORWARD,
		BTR_CFORWARD,
		BTR_LEFT,
		BTR_CLEFT,
		BTR_RIGHT,
		BTR_CRIGHT,
		BTR_ROTR,
		BTR_CROTR,
		BTR_ROTL,
		BTR_CROTL,
		BTR_JUMP,
		BTR_CJUMP,
		BTR_DJUMP,
		BTR_CDJUMP,

		BTR_DEFECT,
		BTR_HMM,
		BTR_IMPOSTER,
		BTR_INVADER,
		BTR_READY,
		BTR_TEST,
		BTR_THISON,
		BTR_WHAT,

		BTR_COUNT, // Don't use
	};

	typedef struct {
		CBotGlitch *m_pGlitch;

		BOOL8 m_bHaveHead;
		BOOL8 m_bHaveTorso;
		BOOL8 m_bHaveLegs;
		BOOL8 m_bStreamBlock;
		BOOL8 m_bHaveMessage;
		BOOL8 m_bHudState;
		BOOL8 _PAD[2];

		CFVec3A m_HeadStartPos;
		CFVec3A m_TorsoStartPos;
		CFVec3A m_LegsStartPos;

		CFQuatA m_HeadStartQuat;
		CFQuatA m_TorsoStartQuat;
		CFQuatA m_LegsStartQuat;

		CFMtx43A m_HeadMtx;
		CFMtx43A m_TorsoMtx;
		CFMtx43A m_LegsMtx;
		CFMtx43A m_Listener;
	} SpyCommonData_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Private definitions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	typedef enum {
		STAGE_CRANE,
		STAGE_MOVE,
		STAGE_SEARCH,
		STAGE_DANCE,
		STAGE_BATTLE,
		STAGE_PROGRAM,
	} SpyVsSpyStage_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Public data 
//----------------------------------------------------------------------------------------------------------------------------------
public:
//	static CBotGlitch *m_pGlitch;
//
//	static BOOL8 m_bHaveHead;
//	static BOOL8 m_bHaveTorso;
//	static BOOL8 m_bHaveLegs;
//	static BOOL8 m_bStreamBlock;
//	static BOOL8 m_bHaveMessage;
//	static BOOL8 m_bHudState;
//	static BOOL8 _PAD[2];
//
//	static CFVec3A m_HeadStartPos;
//	static CFVec3A m_TorsoStartPos;
//	static CFVec3A m_LegsStartPos;
//
//	static CFQuatA m_HeadStartQuat;
//	static CFQuatA m_TorsoStartQuat;
//	static CFQuatA m_LegsStartQuat;
//
//	static CFMtx43A m_HeadMtx;
//	static CFMtx43A m_TorsoMtx;
//	static CFMtx43A m_LegsMtx;
//	static CFMtx43A m_Listener;

	static SpyCommonData_t *m_pCommonData;
	static SpyVsSpyConfigValues_t *m_pConfigValues;
//----------------------------------------------------------------------------------------------------------------------------------
// Private data 
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bLevelInitted;
	static CSpyVsSpy *m_pGame;

	SpyVsSpyStage_e m_eCurrentStage;
	CSpyVsSpyStage *m_pCurrentStage;

	CSpyVsSpyStage *m_pCraneStage;
	CSpyVsSpyStage *m_pMoveStage;
	CSpyVsSpyStage *m_pSearchStage;
	CSpyVsSpyStage *m_pDanceStage;
	CSpyVsSpyStage *m_pBattleStage;
	CSpyVsSpyStage *m_pProgramStage;

	friend class CSpyVsSpyStage;
	friend class CSearchStage;
	friend class CCraneStage;
	friend class CProgramStage;
	friend class CDDRStage;

	FCLASS_STACKMEM_ALIGN( CSpyVsSpy );
} FCLASS_ALIGN_SUFFIX;


//
//
//
// CSpyVsSpySoundCenter
//
//
//
FCLASS_ALIGN_PREFIX class CSpyVsSpySoundCenter {
public:
	static FINLINE CFSoundGroup *GetGroup( const u32 &uNdx ) { FASSERT( uNdx >= 0 && uNdx < SOUND_NUM_SOUNDS ); return m_apGroups[ uNdx ]; }

	static BOOL Load( void );
	static void Unload( void );

public:
	// Order of this enum must match the order of the strings in .cpp
	enum {
		SOUND_CRANE_MOVE_LOOP = 0, 
		SOUND_CRANE_MOVE_START, 
		SOUND_CRANE_MOVE_STOP,
		SOUND_CRANE_PINCH_MISS,
		SOUND_CRANE_PINCH_HIT,
		SOUND_CRANE_ARM_START, 
		SOUND_CRANE_ARM_LOOP,

		SOUND_CONVEYOR_LOOP,
		SOUND_CONVEYOR_SUCCESS_DING,

		SOUND_DISPENSER_DING,
		SOUND_DISPENSER_WORK,

		SOUND_GURNEY_MOVE,
		SOUND_GURNEY_ARM_MOVE_UP,

		SOUND_ELEVATOR_DING,
		
		SOUND_LOCKER_OPEN,
		SOUND_LOCKER_CLOSE,
		SOUND_ARMS_WORK,

		SOUND_COMMAND_LEFT,
		SOUND_COMMAND_RIGHT,
		SOUND_COMMAND_FORWARD,
		SOUND_COMMAND_BACKWARD,
		SOUND_COMMAND_ROTATE_RIGHT,
		SOUND_COMMAND_ROTATE_LEFT,
		SOUND_LEFT,
		SOUND_RIGHT,
		SOUND_FORWARD,
		SOUND_BACKWARD,
		SOUND_ROTATE_RIGHT,
		SOUND_ROTATE_LEFT,

		SOUND_BAD_BOT_WARNING,
		SOUND_DEFECTIVE_BOT,
		SOUND_INSPECT_BOT,
		SOUND_OPEN_LOCKER_WARNING,
		SOUND_IMPOSTOR,
		SOUND_TESTING_READY,
		SOUND_TESTING_START,

		SOUND_1_FINALSPY,
		SOUND_1_UPFORTONIGHT,
		SOUND_1_DROIDHUNT,
		SOUND_1_FUN,
		SOUND_2_LEAVEBOX,
		SOUND_2_DROIDHUNT,
		SOUND_2_BASH,
		SOUND_2_DRINK,

		SOUND_SCRATCH,
		SOUND_BUILD_ARM_SPARKS,
		SOUND_HEAD_CRANE,
		SOUND_PROGRAM_LOOP,
		SOUND_PART_CLATTER,
		SOUND_CHECK_SOUND,

		SOUND_SEARCH_AMBIENT,
		SOUND_PROGRAM_AMBIENT,

		SOUND_DEATH_MACHINE,
		SOUND_IDLE_MACHINE,
		SOUND_BUILD_MACHINE_RUMBLE,

		SOUND_NUM_SOUNDS
	};
protected:
	static CFSoundGroup **m_apGroups;

	FCLASS_STACKMEM_ALIGN( CSpyVsSpySoundCenter );
} FCLASS_ALIGN_SUFFIX;

//
//
//
// CEmitterFader
//
//
//
FCLASS_ALIGN_PREFIX class CEmitterFader {
public:
	static void Setup( void );
	static void Reset( void );
	static void Work( void );
	static void Fade(	CFAudioEmitter *pEmitter,
						const f32 &fStartVol,
						const f32 &fEndVol,
						const f32 &fTimeToFade,
						BOOL bDestroyWhenDone = FALSE,
						const CFVec3A *pPos = NULL );

	static void FadeInAndOut(	CFAudioEmitter *pEmitter,
								const f32 &fStartVol,
								const f32 &fEndVol,
								const f32 &fTimeToFade,
								const f32 &fUnitMaxVol,
								BOOL bDestroyWhenDone = FALSE,
								const CFVec3A *pPos = NULL );

protected:
	enum {
		_MAX_FADERS = 10,
	};

	typedef struct {
		BOOL8 bUsed;
		BOOL8 bKillWhenDone;
		BOOL8 bInAndOut;

		f32 fVolumeStart;
		f32 fVolumeEnd;

		f32 fTime;
		f32 fTimeTotal; // Total time we should work
		f32 fTimeOOTotal; // 1 / fTimeTotal for non-in and out faders and 1 / fTimeTotal * fUnitMaxVol for in and out faders

		const CFVec3A *pPos;
		CFAudioEmitter *pEmitter;
	} FaderEntry_t;

protected:
	static FaderEntry_t *_GetFader( void );

protected:
	static BOOL m_bAlive;
	static FaderEntry_t m_Faders[ _MAX_FADERS ];

	FCLASS_STACKMEM_ALIGN( CEmitterFader );
} FCLASS_ALIGN_SUFFIX;


#endif

