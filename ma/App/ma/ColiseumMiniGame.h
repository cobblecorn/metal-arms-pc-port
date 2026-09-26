//////////////////////////////////////////////////////////////////////////////////////
// ColiseumMiniGame.h - Actor type of camera.
//
// Author: Jeremy Chernobieff
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
// 04/01/02 Chernobieff	Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _COLISEUM_MINIGAME_H_
#define _COLISEUM_MINIGAME_H_ 1

// =============================================================================================================

#include "fang.h"
#include "fmath.h"
#include "fparticle.h"

#include "collectable.h"
#include "camsimple.h"

#include "level.h"
#include "MagnetDevice.h"
#include "ai\aicorrosive.h"

class CBot;
class CBotGrunt;
class CWeapon;
class CMeshEntity;
class CFWorldMesh;
class CFAudioEmitter;
class CFDebrisDef;

//enum AIBehaviorReason_e;

FCLASS_ALIGN_PREFIX class CColiseumMiniGame
{
public:
	CColiseumMiniGame();
//	~CColiseumMiniGame();

	static BOOL LoadLevel( u32 nLevel );
	static BOOL LoadLevel1( LevelEvent_e eEvent );
	static BOOL LoadLevel2( LevelEvent_e eEvent );
	static BOOL LoadLevel3( LevelEvent_e eEvent );
	static BOOL LoadLevel4( LevelEvent_e eEvent );
	static BOOL LoadMP( LevelEvent_e eEvent );
	static void UnloadMP( void );

	static void UnloadLevel( void );
	static void Work( void );
	static void WorkMP( void );
	static void Restore( void );

	enum
	{
		SCORE_MINIMUM = 1,
		MAX_MAGNET_DEVICES = 24,
		MAX_FRIENDLY=10,
		MAX_ENEMIES=30,
		MAX_AMMO=20,
		MAX_TRIPWIRES=20,
		MAX_FRONT_CROWD=100, //70
		MAX_FAKE_CROWD=100,
		MAX_WEAPONS=10,
	#if FANG_PLATFORM_GC
		MAX_CROWD_TEMPLATE=6,
	#else
		MAX_CROWD_TEMPLATE=15,
	#endif
		MAX_SOUNDS_PLAYING=16
	};

	typedef enum
	{
		STATE_INIT=0,  //init the player
		STATE_ESCORT,
		STATE_WEAPON_SELECT,
		STATE_ESCORT_OUT,
		STATE_MAIN_GAME,
		STATE_END_CUTSCENE,
		STATE_END_GAME,
		STATE_WAIT,

		NUM_STATES
	} MiniGameStates_e;

	typedef enum
	{
		COLISEUMSND_COMMON_BLADE=0,
		COLISEUMSND_COMMON_MAGNET,
		COLISEUMSND_COMMON_BOO_L01,
		COLISEUMSND_COMMON_BOO_L02,
		COLISEUMSND_COMMON_BOO_M01,
		COLISEUMSND_COMMON_BOO_M02,
		COLISEUMSND_COMMON_BOO_S01,
		COLISEUMSND_COMMON_BOO_S02,
		COLISEUMSND_COMMON_CHEER_L01,
		COLISEUMSND_COMMON_CHEER_L02,
		COLISEUMSND_COMMON_CHEER_L03,
		COLISEUMSND_COMMON_CHEER_M01,
		COLISEUMSND_COMMON_CHEER_M02,
		COLISEUMSND_COMMON_CHEER_M03,
		COLISEUMSND_COMMON_CHEER_S01,
		COLISEUMSND_COMMON_CHEER_S02,
		COLISEUMSND_COMMON_CHEER_S03,
		COLISEUMSND_COMMON_OWW_L01,
		COLISEUMSND_COMMON_OWW_L02,
		COLISEUMSND_COMMON_OWW_M01,
		COLISEUMSND_COMMON_OWW_M02,
		//Music "Stingers"
		COLISEUMSND_COMMON_STINGER_A1,
		COLISEUMSND_COMMON_STINGER_A2,
		COLISEUMSND_COMMON_STINGER_A3,
		COLISEUMSND_COMMON_STINGER_B1,
		COLISEUMSND_COMMON_STINGER_B2,
		COLISEUMSND_COMMON_STINGER_B3,
		COLISEUMSND_COMMON_STINGER_C1,
		COLISEUMSND_COMMON_STINGER_C2,
		COLISEUMSND_COMMON_STINGER_C3,

		COLISEUMSND_COMMON_COUNT,
	
		COLISEUMSND_LEVEL_ANN01=COLISEUMSND_COMMON_COUNT,
		COLISEUMSND_LEVEL_ANN02,
		COLISEUMSND_LEVEL_ANN03,
		COLISEUMSND_LEVEL_ANN04,
		COLISEUMSND_LEVEL_ANN05,
		COLISEUMSND_LEVEL_ANN06,
		COLISEUMSND_LEVEL_ANN07,
		COLISEUMSND_LEVEL_CAL_01,
		COLISEUMSND_LEVEL_CAL_02,
		COLISEUMSND_LEVEL_CAL_03,
		COLISEUMSND_LEVEL_CAL_04,
		COLISEUMSND_LEVEL_CAL_05,
		COLISEUMSND_LEVEL_CAL_06,
		COLISEUMSND_LEVEL_CAL_07,
		COLISEUMSND_LEVEL_CAL_08,
		COLISEUMSND_LEVEL_CAL_09,
		COLISEUMSND_LEVEL_CAL_10,
		COLISEUMSND_LEVEL_JER_01,
		COLISEUMSND_LEVEL_JER_02,
		COLISEUMSND_LEVEL_JER_03,
		COLISEUMSND_LEVEL_JER_04,
		COLISEUMSND_LEVEL_JER_05,
		COLISEUMSND_LEVEL_JER_06,

		COLISEUMSND_LEVEL_CROWD,

		COLISEUMSND_COUNT,
	} ColiseumSounds_e;

	static BOOL Targeting( const CFVec3A *pRayStart_WS, const CFVec3A *pRayEnd_WS, CFVec3A *TargetedPointOnTarget_WS, CFWorldMesh **pTargetedTargetWorldMesh );
		
private:

	typedef enum
	{
		SCORE_NONE=0,
		SCORE_DIRECT,
		SCORE_TRAP,
		SCORE_GRENADE,
		SCORE_DECAPITATE,

		SCORE_NUM_TYPES
	} ScoreType_e;

	typedef enum
	{
		CHEER_MAX,
		CHEER_LARGE,
		CHEER_MED,
		CHEER_SMALL
	} CheerLevel_e;

	typedef enum
	{
		BOO_MAX,
		BOO_MED,
		BOO_SMALL
	} BooLevel_e;
	
	static CColiseumMiniGame *m_pColiseumMiniGame;

	FSndFx_FxHandle_t m_hSounds[COLISEUMSND_COUNT];
	CFAudioEmitter *m_pSoundEmitters[COLISEUMSND_COUNT];

	s32 m_nLevelID;
	MiniGameStates_e m_State;
	s32 m_nNumEnemies;
	s32 m_nNumFriendly;
	s32 m_nNumAmmo;
	s32 m_nScore, m_nMilScore;
	s32 m_nMagnetDevices;
	s32 m_nNumSoundEmitters;
	s32 m_nNumFrontCrowd;
	s32 m_nNumFakeCrowd;
	s32 m_nNumSoundsPlaying;

	CFTexInst *m_pTVScrTex;
	CFTexInst *m_pCrowdTex[MAX_CROWD_TEMPLATE];
	CFTexInst *m_pCrowdAlpha[MAX_CROWD_TEMPLATE];

	u8 m_auCrowdIdx[MAX_FAKE_CROWD*5];
	s8 m_anCrowdOffs[MAX_FAKE_CROWD*5];

	CEntity *m_pAmmoPlacement[MAX_AMMO];
	CBot *m_pEnemies[MAX_ENEMIES];
	CBot *m_pFriendly[MAX_FRIENDLY];
	CBot *m_pGarbageGrunt[4];
	CFWorldMesh *m_pScoreBoard[4];
	CFWorldMesh *m_pTVScr[4];
	CFWorldMesh *m_pFrontCrowd[MAX_FRONT_CROWD];
	CFWorldMesh *m_pFakeCrowd[MAX_FAKE_CROWD];

	CDoorEntity *m_pCorrosiveThrone;

	BOOL m_bHitWhole;
	//TV Screen stuff
	CFVec3 m_vCamPos[2];
	CFVec3 m_vCamTarget[2];
	CFVec3 m_vLastCamLookAt;
	s32 m_nCamIdx;
	f32 m_fOffsX, m_fMaxOffsX, m_fMinOffsX, m_fDeltaX;
	f32 m_fCamScale, m_fDeltaScale, m_fMaxScale;
	f32 m_fSwitchTime, m_fBotSwitch;
	s32 m_nCurBot;
	//Corrosive Stuff (Coliseum 3)
	f32 m_fFireTime;
	//Corrosive Stuff (Coliseum 4)
	u32 m_nNumTripwires;
	u32 m_nNumHuts;
	CEntity *m_paTripwireEntity[MAX_TRIPWIRES];
	CEntity *m_pTripwireInside;
	CMeshEntity *m_pHutInside;
	s32 m_nHitIdx;
	CMeshEntity *m_paHuts[MAX_TRIPWIRES];
	//Garbage throwing grunts
	FMesh_t *m_pGarbageMesh[4];
	CFDebrisDef m_DebrisDef;

	static CInventory m_InvCopy;

	//Sound & Camera switch
	BOOL m_bPlayBladeSnd;
	BOOL m_bPlayMagnetSnd;
	BOOL m_bSwitched;

	s32 m_anWpnIdx[MAX_WEAPONS];
	CWeapon *m_apWeapons[MAX_WEAPONS];
	CWeapon *m_apStoredWpn[64];
	s32 m_nNumWeapons;
	s32 m_nNumStoredWpn;

	f32 m_fTimeUntilDec;
	u8 m_nExcitedLevel;
    
	FParticle_DefHandle_t m_hWeaponParticleDef;
	//Magnet Device traps
	CMagnetDevice *m_apMagnetDevice[MAX_MAGNET_DEVICES];
	
	s32 m_aTexLayerHandle[8];
	s32 m_aTVLayerHandle[4];
	f32 m_fScored;
	f32 m_fScoredMil;
	f32 m_fTimeSinceLastDamage;
	f32 m_fCutSceneTime;
	static const f32 m_fCutSceneLength[4];
	ScoreType_e m_eScoreType;

	CFVec2 m_vScore10, m_vScore1;

	f32 m_fStreamVolume;
	f32 m_fMaxStreamVolume;
	CFAudioStream *m_pAudioStream;

	CFXfm m_VXfm;
	CFVec3A m_tmpVec3A;
	FViewport_t *m_pSView;
	BOOL m_bDoPeer;
	BOOL m_bDoFlickNextFrame;

	//Sound tracking.
	u8 m_nLastCheerIdx;
	u8 m_nLastBooIdx;
	u8 m_nLastStingerIdx;
	u8 m_nLastAnnouncerIdx;
	u8 m_nLastCallIdx;
	u8 m_nLastJeerIdx;
	s8 m_nLastTexIdx;
	u8 m_nGarbageIdx;
	f32 m_fJeerTime;
	f32 m_fWaitTimer;
	//

	CFMtx43A m_dMtx;

	//Cutscene variables
	CFVec3A m_vDesLoc, m_vDesDir, m_vCurLoc, m_vStartLoc;
	CCamSimpleInfo m_CamManSimpleInfo;
	CFQuatA m_qCamera;
	//

	CFSoundGroup *m_pDebrisSndGroup;
	
	//**********private functions************
	//Callbacks
	static void MagnetDeviceCallback(void *pUserData, void *pBot, CFVec3& vPos);
	static void MagnetFXCallback(void *pUserData, void *pSpd, CFVec3& vPos);
	static void TvScreenCallback(void *pUserData);
	static void CrowdCallback(void *pUserData);
	static void CheckTripWireEvents(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);
	static void HutDestroyedCallback(u32 nUserData);
	static BOOL PeerDone( AIBehaviorReason_e eReason );
	static void CorrosiveDamageCallback(void);
	static void CorrosiveRoarCallback(u32 nUserData);
	static void CorrosiveRangeNoRocketsCallback(u32 nUserData);
	
	FExplosion_GroupHandle_t m_hExpGroup;

	//Game play
	void InitGame();
	BOOL Escort();
	BOOL WeaponSelect();
	BOOL EscortOut();
	BOOL MainGame();
	BOOL HandleEndCutscene();
	void EndGame();
	void SetupTripwires();
	void UpdateScoreBoard();

	//Grunt garbage throwing
	void GruntsThrowGarbage();

	//Utility
	BOOL IsCurBotDead();
	BOOL RegisterForTripwireEvents();
	void CheckTripWireEvents();
	CWeapon *CreateWeapon( CollectableType_e eType );
	void CrushParent(CEntity *pParent);

	//Sound playing, used for crowd reactions & trap SFX
	BOOL PlaySnd2D(ColiseumSounds_e eSnd, BOOL bPlay, BOOL bOverride=TRUE, f32 fVolumeMul=1.0f);
	BOOL PlaySnd3D(ColiseumSounds_e eSnd, CFVec3A *pPos_WS, f32 fOuterRadius, f32 fVolumeMul=1.0f, f32 fFreqMul=1.0f, BOOL bPlay=TRUE, BOOL bOverride=TRUE);
	BOOL StopSnd(ColiseumSounds_e eSnd);
	void SetSndPan(ColiseumSounds_e eSnd, f32 fPan=0.0f);
	void StopAllSounds();

	void PlayCheer(CheerLevel_e eCheer);
	void PlayBoo(BooLevel_e eBoo);
    void PlayAnnouncer();
	void PlayMusicSinger();
	void PlayCall();
	void PlayJeer(BOOL bRndPan=FALSE);
	void PlayPlayerDeathSound();
	void UpdateCrowdAnim();
	void CleanUpSounds();

    FCLASS_STACKMEM_ALIGN(CColiseumMiniGame);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
