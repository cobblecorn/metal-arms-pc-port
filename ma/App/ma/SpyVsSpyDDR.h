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
// 04/21/03 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SPYVSSPYDDR_H_
#define _SPYVSSPYDDR_H_ 1

#include "SpyVsSpy.h"
#include "botminer.h"

class CESpline;
class CBotGrunt;
class CMeshEntity;
class CCamSimple;

typedef enum {
	MOVE_TYPE_INVALID = 0,
	// Don't mess with the order, it matters
	MOVE_TYPE_LEFT,
	MOVE_TYPE_RIGHT,
	MOVE_TYPE_FORWARD,
	MOVE_TYPE_BACKWARD,
	MOVE_TYPE_ROTATE_RIGHT,
	MOVE_TYPE_ROTATE_LEFT,
	MOVE_TYPE_JUMP,
	MOVE_TYPE_DOUBLE_JUMP,
	MOVE_TYPE_COUNT, 
} DanceMoveType_e;

FCLASS_NOALIGN_PREFIX class CDanceMoveTable { 
public:
	CDanceMoveTable() { Reset(); }

	BOOL Setup( u32 uNumGroups );
	void Reset( void );

	u32 GetDanceMove( u32 uGrp, u32 uMove ) const	{ FASSERT( uGrp < m_uNumMoveGroups ); FASSERT( uMove < m_pauNumMovesPerGroup[ uGrp ] ); return m_paeMoves[ uGrp ][ uMove ]; }
	u32 GetNumMoves( u32 uGrp ) const							{ FASSERT( uGrp < m_uNumMoveGroups ); return m_pauNumMovesPerGroup[ uGrp ]; }

public:
	u32 m_uNumMoveGroups;
	u32 *m_pauNumMovesPerGroup;
	u32 **m_paeMoves;

	FCLASS_STACKMEM_NOALIGN( CDanceMoveTable );
} FCLASS_NOALIGN_SUFFIX;

FCLASS_NOALIGN_PREFIX class CDanceMoveWalker { 
public:
	void Reset( void );

public:
	u32 m_uMoveGroup;
	u32 m_uCurrentMove;
	u32 m_uNumFakeMoves;

	FCLASS_STACKMEM_NOALIGN( CDanceMoveWalker );
} FCLASS_NOALIGN_SUFFIX;

//
//
//
// CDDRStage - 
//
//
//
FCLASS_ALIGN_PREFIX class CDDRStage : public CSpyVsSpyStage { 
//----------------------------------------------------------------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CDDRStage();
	virtual ~CDDRStage() { }

	BOOL Load( LevelEvent_e eEvent );
	void Unload( void );
	void Restore( void );
	void Work( void );
	void SwitchTo( void );

	void TossSparksFromBot( CBot *pBot );

//----------------------------------------------------------------------------------------------------------------------------------
// Private definitions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum {
		NUM_MINERS = 2,
		NUM_TVS = 7
	};

	enum {
		FLAG_HAVE_JUMP				= 0x0001,	// TRUE if we have a jump command
		FLAG_JUMP1					= 0x0002,	// TRUE if we have a jump 1 command
		FLAG_JUMP2					= 0x0004,	// TRUE if we have a jump 2 command
		FLAG_JUMP_OK				= 0x0008,	// TRUE if the jump command was executed
		FLAG_MOVE_COMMAND			= 0x0020,	// TRUE if we have a move command
		FLAG_WAIT_EXECUTE			= 0x0040,	// TRUE if we are waiting to execute a command
		FLAG_FAKE_COMMAND			= 0x0080,	// TRUE if we have a fake command
		FLAG_JUMP1_IN_QUEUE			= 0x0100,	// TRUE if we have a jump 1 in the queue, but the guy is still talking
		FLAG_JUMP2_IN_QUEUE			= 0x0200,	// TRUE if we have a jump 2 in the queue, but the guy is still talking
		FLAG_STOP_CAM_ANIM			= 0x0400,	// TRUE if we are done animating the camera
		FLAG_SPECIAL_WATCH_MODE		= 0x0800,	// TRUE if we are in the special watch mode.  We watch the DDR, but don't take part
		FLAG_SPECIAL_FAIL_COMMANDS	= 0x1000,	// TRUE if we are in special mode and want the bot to fail his commands
		FLAG_DELAY_MESH				= 0x2000,	// TRUE if we want to delay the destination mesh

		// Don't include FLAG_SPECIAL_WATCH_MODE or FLAG_SPECIAL_FAIL_COMMANDS in here
		FLAG_ALL_BITS		=	FLAG_HAVE_JUMP | FLAG_JUMP1 | FLAG_JUMP2 | FLAG_JUMP_OK | FLAG_MOVE_COMMAND |
								FLAG_WAIT_EXECUTE | FLAG_FAKE_COMMAND | FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE |
								FLAG_DELAY_MESH | FLAG_STOP_CAM_ANIM,
		FLAG_SPECIAL_BITS	= FLAG_SPECIAL_WATCH_MODE | FLAG_SPECIAL_FAIL_COMMANDS,
	};

	typedef enum {
		STATE_WAIT_DIALOG,
		STATE_WAIT_WORK,
		STATE_GIVE_COMMAND,
		STATE_EXECUTE_COMMAND,
		STATE_WAIT_FINISH_COMMAND,
		STATE_IDLE_TIME,
		STATE_CHECK_GLITCH,
		STATE_WALK_GLITCH_BACK,
		STATE_RUN_TO_DOORS,

		STATE_GLITCH_GETS_KILLED,
		STATE_GRAB_GLITCH,

		STATE_DDR_DONE,
	} StageState_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	FINLINE BOOL _IsInSpecialMode( void ) { return ( m_uFlags & FLAG_SPECIAL_WATCH_MODE ); }

	void _ClearDataMembers( void );

	void _RelocateBots( void );
	void _InitWalker( void );

	void _IssueCommand( void );
	void _ExecuteCommand( void );
	void _TellAIToMove( u32 uType, CFMtx43A *pMtx = NULL ); // If pMtx is non-null, we consider this a fake move
	void _RotateDirections( u32 uType, CFMtx43A *pMtx = NULL ); // If pMtx is non-null, we consider this a fake move
	void _IssueJump( u32 uType );
	void _FakeMove( u32 uFake, BOOL bAudioOnly = FALSE );

	void _CheckBotsDone( void );
	void _RecalcLookPoint( void );
	void _CheckAlignment( CBot *pBot );
	
	void _MeshAlphaWork( void );
	void _MeshPlaceIntoWorld( CFMtx43A *pMtx = NULL, BOOL bJump = FALSE, BOOL bDblJump = FALSE, BOOL bDelay = FALSE );

	void _CheckOnGlitch( void );
	void _CheckGlitchWork( void );
	void _GlitchMessedUp( void );

	void _SetGlitchGetsKilled( void );
	void _SetStateGrabGlitch( void );
    void _CraneWork( void );

	void _RunToDoors( void );
	void _RunToDoorsWork( void );

	static void _TVCallback( void *pUserData );

	BOOL _LoadCSV( void );
	BOOL _LoadFromGameData( FGameDataTableHandle_t hTable, u32 uTableNum );

#if !FANG_PRODUCTION_BUILD && SAS_ACTIVE_USER == SAS_USER_NATHAN
	void _DrawDebug( void );
	void _DrawDebugGrid( void );
#endif

//----------------------------------------------------------------------------------------------------------------------------------
// Private data
//----------------------------------------------------------------------------------------------------------------------------------
private:
	u32 m_uFlags;
	StageState_e m_eStageState;

	f32 m_fTimeInterp;
	f32 m_fIdleTimer;
	f32 m_fMoveTimeIdle;

	f32 m_fAlignFailTimer;
	f32 m_fPositionFailTimer;
	f32 m_fLookFailTimer;

	CFMtx43A m_DirMatrix;
	CFVec3A m_LookPoint;
	CFMtx43A m_StartMatrix;

	CBotMiner *m_pKillMiner;
	CBotMiner *m_paMiners[ NUM_MINERS ];
	CFVec3A m_StartPoints[ NUM_MINERS ];
	CFVec3A m_DestPoints[ NUM_MINERS ];

	f32 m_fGlitchMoveTimer;
	CFVec3A m_GlitchStartPoint; // Where Glitch started the level
	CFVec3A m_GlitchDestPoint;

	CDanceMoveTable m_DanceTable;
	CDanceMoveWalker m_DanceMoveWalker;

	f32 m_fHoloMeshAlpha;
	f32 m_fHoloMeshAlphaFadeDir;
	CFWorldMesh *m_pHoloMesh;

	u32 m_uBadMoveCount;
	CFVec3A m_CommandGruntStart;
	CBotGrunt *m_pCommandGrunt;

	CFVec3A m_DoorDest1;
	CFVec3A m_DoorDest2;
	CFVec3A m_DoorDest3;

	u32 m_uGruntSplinePoint;
	CESpline *m_pRunSpline;

	f32 m_fUnitCamInterp;
	CFQuatA m_StartQuat;
	CCamSimpleInfo m_Camera;
	CFVec3A *m_pCraneBone;
	CMeshEntity *m_pDDRCrane;

	CFVec3A m_BattleStartPoint;
	CFVec3A m_CraneGrabPoint;

	CFVec3A m_GotoPoint;

	FViewport_t *m_pViewport;
	CFTexInst *m_pTVTexture;

	FParticle_DefHandle_t m_hSparkParticleDef;

	CMeshEntity *m_pCrusher;
	CFAudioEmitter *m_pCrusherEmitter;

	friend class CSearchStage;

	FCLASS_STACKMEM_ALIGN( CDDRStage );
} FCLASS_ALIGN_SUFFIX;

#endif