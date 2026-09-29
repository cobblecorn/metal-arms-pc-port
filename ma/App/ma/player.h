//////////////////////////////////////////////////////////////////////////////////////
// player.h - Human player module.
//
// Author: Steve Ranck     
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
// 03/29/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYER_H_
#define _PLAYER_H_ 1

#include "fang.h"
#include "fviewport.h"
#include "reticle.h"
#include "entitycontrol.h"
#include "iteminst.h"
#include "ftext.h"
#include "skybox.h"
#include "level.h"
#include "Hud2.h"
#include "vehicle.h"

enum { MAX_PLAYERS = 4 };

#define PLAYER_TARGET_ASSIST_RETICLE_MULTIPLIER		0.7f		// The reticle's screen size is multiplied by this to determine the size of the fire steering zone
#define PLAYER_TARGET_ASSIST_TRACKER_MULTIPLIER		0.75f		// The tracker's screen size is multiplied by this to determine intersection with the fire steering zone
#define PLAYER_AIM_BIASING_RETICLE_MULTIPLIER		3.0f		// Aim biasing is to be applied in a screen area this many times the size of the reticle (as adjusted by PLAYER_TARGET_ASSIST_RETICLE_MULTIPLIER)
#define PLAYER_AIM_BIASING_MESH_SAMPLES				2			// Number of bias mesh screen space velocity samples that are taken for determining bias
#define PLAYER_MINIMUM_YAW_BIAS_WEIGHT				0.5f		// 1.f would cause the reticle to attempt to stay locked on the target
#define PLAYER_MINIMUM_PITCH_BIAS_WEIGHT			0.4f		// 1.f would cause the reticle to attempt to stay locked on the target

class CEntity;
class CPlayerProfile;
class CVehicle;
struct GameInitInfo_t;

// Stats needed for single player
struct Player_SPStats_t {
	u32 m_nUnitsDestroyed;					// how many units did the player kill
	u32 m_nWashersCollected;				// how many washers did the player pickup during this level
	u32 m_nSecretAreasUncovered;			// how many secret areas where uncovered during this level
	u32 m_nSecretChipsCollected;			// how many secret chips were collected on this level

};

// Stats needed for multi-player
struct Player_MPStats_t {
	s32		m_nKills;			// How many players I killed this game (can be negative)
	u32		m_nDeaths;			// How many times I died this game
};

// Game stats. We only need one type per round, so union them together
struct Player_GameStats_t {
	union {
		Player_SPStats_t m_SPStats;
		Player_MPStats_t m_MPStats;
	};
	BOOL8	m_bIsSinglePlayer;		// Just so the structure describes itself
	void InitLevel( BOOL bSinglePlayer );
};


class CPlayer {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		PF_HAS_ENTITY_CONTROL		= 0x00000001,	// The player may or may not actually be in control of his current entity (AI or Cinematic may be)
		PF_FADING_VIEW				= 0x00000002,	// TRUE when fading view
		PF_TEXT_BOX_VISIBLE_SAVE1	= 0x00000004,	// Used to save the value of the bVisible attribute in m_hTextBoxOutOfRange during the pause screen
		PF_TEXT_BOX_VISIBLE_SAVE2	= 0x00000008,	// Used to save the value of the bVisible attribute in m_hTextBoxScopeZoom and m_ahTextBoxScopeInfo during the pause screen
		PF_TEXT_BOX_VISIBLE_SAVE3	= 0x00000010,	// Used to save the value of the bVisible attribute in m_hTextBoxLowNoAmmo during the pause screen
		PF_TEXT_BOX_VISIBLE_SAVE4	= 0x00000020,	// Used to save the value of the bVisible attribute in m_hTextBoxRestart during the pause screen
		PF_TEXT_REMOVED				= 0x00000040,	// The text has been removed
		PF_DONT_CALL_WORK			= 0x00000080,	// Used to not call the entity work function

		PF_NONE						= 0x00000000
	};


	typedef void DrawOverlayFcn_t( u32 nPlayerIndex, void *pUser );




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static s32 m_nPlayerCount;				// Number of human players in the current game (0=none for attract mode)
	static s32 m_nCurrent;					// Current player number (-1 = none)
	static CPlayer *m_pCurrent;				// Current player we're working on (NULL = none)
	
	static f32 m_fMusicVolumeCache; // -1 not cached, otherwise volume that should be saved to profile instead of asking the audio system (cause we probably faded it out)
	static f32 m_fSfxVolumeCache;	// -1 not cached, otherwise volume that should be saved to profile instead of asking the audio system (cause we probably faded it out)
		
	u32 m_nPlayerIndex;						// Player index from 0 to MAX_PLAYERS-1
	u32 m_nPlayerNum;						// Player number from 1 to MAX_PLAYERS
	u32 m_nControllerIndex;					// Controller index this player is using
	u32 m_nTeamNum;							// Which team is this player on (mult-player only for now)

	FViewport_t *m_pViewportPersp3D;		// The player's Persp3D viewport
	FViewport_t *m_pViewportOrtho3D;		// The player's Ortho3D viewport
	FViewport_t *m_pViewportSafeOrtho3D;	// The player's Ortho3D viewport, shrunk to fit inside safe area

	CHud2			m_Hud;					// This player's hud
	CReticle		m_Reticle;				// This player's reticle
	CHumanControl	m_HumanControl;			// The human control input data

	CEntity *m_pEntityOrig;					// This player's original entity (usually a CBot)
	CEntity *m_pEntityCurrent;				// This player's current entity (vehicle, possessed Mil, etc.)
	u32	m_uPlayerFlags;						

	FTextAreaHandle_t m_hTextBoxRestart;	// Used to display the "Press A to Continue" message when the player has been destroyed
	FTextAreaHandle_t m_hTextBoxLowNoAmmo;	// Used to display the low/no ammo messages (used by CReticle)
	FTextAreaHandle_t m_hTextBoxOutOfRange;	// Used to display the possession out-of-range warning (used by CBot)
	FTextAreaHandle_t m_hTextBoxScopeZoom;	// Used to display the scope zoom magnification (used by CReticle)
	FTextAreaHandle_t m_ahTextBoxScopeInfo[RETICLE_SCOPE_INFO_SLOT_COUNT];

	CSkyBox m_SkyBox;						// The sky box for this player

	CPlayerProfile *m_pPlayerProfile;		// pointer to player profile data object
	cwchar *m_pwszPlayerName;				// the name of this player, either their profile name or an assigned name if there was a duplicate or no profile provided

	BOOL8 m_bHidePlayerEntity;				// Default: FALSE (TRUE=hide the current entity when rendering this player's viewpoint)
	BOOL8 m_bInvertLook;					// Default: FALSE  (TRUE=Invert look on Y-axis FALSE=No inverted look on Y-axis)
	BOOL8 m_bAutoCenter;					// Default: FALSE  (TRUE=Auto-center the view FALSE=Don't auto-center the view)
	BOOL8 m_bFourWayQuickSelect;			// TRUE=enable 4-way quick-select
	BOOL  m_bForceReady;
	f32  m_fUnitTargetingAssistance;		// Default: 1.0f   (1.0=Max, 0.0=None)
	f32  m_fLookSensitivity;				// Default: 1.0f   (Looking sensitivity(1.0=Max, 0.0=Min))

	CVehicle *m_pRacingVehicle;				// if not NULL, then player should always be inside this vehicle (for racing levels)

	// Target assist parameters
	BOOL  m_bReticleCenterOverTarget;		// If TRUE, a ray cast from the cam through the reticle hits the current targeted entity

	// Aim biasing parameters (these parameters will cause subtle adjustments to the player's orientation)
	const CFWorldMesh *m_pBiasingMesh;
	CFVec3 m_vBiasMeshPosLastFrame;
	f32	   m_afFrameYawVel[PLAYER_AIM_BIASING_MESH_SAMPLES];
	f32    m_afFramePitchVel[PLAYER_AIM_BIASING_MESH_SAMPLES];
	f32    m_fYawVelAvg;
	f32    m_fPitchVelAvg;
	f32    m_fYawAdjust;
	f32    m_fPitchAdjust;
	u8     m_nCurrentSample;
	u8	   m_nMultiplayerColor;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;
	static CInventory *m_pInvSave;
	
	static f32 m_fStartOfLevelMusicVol;
	static f32 m_fStartOfLevelSfxVol;

	CInventory m_BotInv;					// The inventory for the normal player bot (ie, Blink).
	CInventory m_PosInv;					// Temporary inventory to be initialized when the player is temporarily
											//   controlling another bot (possession or otherwise).

	DrawOverlayFcn_t *m_pFcnDrawOverlay;	// Draw-overlay function (NULL=none)
	void *m_pDrawOverlayUser;				// User parameter for draw-overlay function

	CFColorRGBA m_ViewFadeStartColorRGBA;	// Start fade color
	CFColorRGBA m_ViewFadeEndColorRGBA;		// End fade color
	f32 m_fUnitFade;						// Amount of fade so far
	f32 m_fDeltaUnitFade;					// Rate of fade

	// player stats that are tracked during gameplay
	Player_GameStats_t	m_GameStats;

	// Variables used to determine if player is stuck in the 
	// air and thus should be returned to a check point
	u8 m_nInDebugCameraResetDelay;
	BOOL8 m_bRestoreTimerSuspended;
	BOOL8 m_bRestoreTimerRunning;			// TRUE if the respawn timer is currently running
	u8 __PAD;
	f32  m_fRestoreCheckpointTimer;			// Number of seconds the respawn timer has been running 
	CFVec3 m_vPosWhenLeavingGround;			// Player position when he first becomes "in air"


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }

	static BOOL InitLevel( const GameInitInfo_t *pGameInit, Level_e nLevel );
	static void UninitLevel( Level_e nLevel, BOOL bLevelCompleted );

	static void SetCurrent( s32 nCurrentPlayerIndex );
	static CInventory *GetInventory( s32 nPlayerIndex, BOOL bPossession = FALSE );
	static void AbortAllPlayerScopeZoomMode( BOOL bImmediate );

	// Total team scores
	static s32 GetTeamScores(s32* pnKills, u32* pnDeaths);

	FINLINE static void SetSaveInventory(CInventory *pInv) { m_pInvSave = pInv; }

	void Work( void );
	void HandleTogglingOfOnscreenText( void );
	BOOL HasEntityControl(void)					{ return (BOOL)(m_uPlayerFlags & PF_HAS_ENTITY_CONTROL);};
	void DisableEntityControl(void);
	void EnableEntityControl(void);
	void ZeroControls( void );
	FINLINE BOOL IsPossessingMil(void)			{ return m_pEntityOrig != m_pEntityCurrent && m_pEntityOrig && m_pEntityCurrent;}
	void ReturnToOriginalBot(void);
	void ResetTargetAssistance( void );

	// Bring this bot back to life in preparation for respawn or checkpoint restore
	void Resurrect( void );

#if FANG_WINGC
	// Local co-op: a player who goes down stays down while a partner stands; the next checkpoint
	// brings them back beside a partner (called just before the checkpoint is saved).
	static void CoopReviveForCheckpoint( void );
	BOOL CoopWaitingForCheckpoint( void );
#endif
	
	// Call just before/after rendering this player's main view
	void PreRender(void);
	void PostRender(void);

	// Draw any player-specific text not handled by Bot functions
	void DrawText(void);

	FINLINE DrawOverlayFcn_t *GetDrawOverlayFunction( void ) const { return m_pFcnDrawOverlay; }
	FINLINE void SetDrawOverlayFunction( DrawOverlayFcn_t *pFcnDrawOverlay, void *pUser=NULL ) { m_pFcnDrawOverlay = pFcnDrawOverlay; m_pDrawOverlayUser=pUser; }

	void StartViewFade( CFColorRGBA *pStartColorRGBA, CFColorRGBA *pEndColorRGBA, f32 fFadeTime );
	void StopViewFade( void );
	FINLINE BOOL IsViewFading( void ) const { return (BOOL)(m_uPlayerFlags & PF_FADING_VIEW); }
	FINLINE f32 GetViewFadeUnitProgress( void ) const { return m_fUnitFade; }

	void SetInvertLook( BOOL bInvert )			{ m_bInvertLook = bInvert; }
	BOOL GetInvertLook( void )					{ return m_bInvertLook; }

	void SetAutoCenter( BOOL bCenter ) { m_bAutoCenter = bCenter; }
	FINLINE BOOL GetAutoCenter( void ) const { return m_bAutoCenter; }

	void SetFourWayQuickSelect( BOOL bFourWayQuickSelect ) { m_bFourWayQuickSelect = bFourWayQuickSelect; }
	FINLINE BOOL GetFourWayQuickSelect( void ) const { return m_bFourWayQuickSelect; }

	void SetTargetingAssistance( BOOL bAssist )	{ if ( bAssist ) m_fUnitTargetingAssistance = 1.f; else m_fUnitTargetingAssistance = 0.f; }
	FINLINE f32 GetTargetingAssistance( void ) const { return m_fUnitTargetingAssistance; }

	void SetLookSensitivity( f32 fSen ) { m_fLookSensitivity = fSen; }
	FINLINE f32 GetLookSensitivity( void ) const { return m_fLookSensitivity; }
	f32 ComputeLookSensitivityMultiplier( void ) const;

	BOOL SetInVehicle( CVehicle *pVehicle, CVehicle::Station_e eStation );		// place player in vehicle for duration of level

	void SetHiddenFromSelf(BOOL bHidden)		{ m_bHidePlayerEntity = bHidden; }

	// For setting game stats
	void CreditKill( s32 nDeadPlayer, const CEntity* pWhat );
	void CreditWasher( void );
	void CreditDeath( void );
	void CreditSecretArea( void );
	void CreditSecretChip( void );

	// Retrieving game stats

	// Score for one player
	void GetMPScore(s8& nKills, u8& nDeaths);

	void GetSPStats( u32 &rnUnitsDestroyed, u32 &rnWashersCollected );
	void OverrideUnitsDestroyed( u32 nUnitDestroyed );

	BOOL IsReadyToContinue( void ); // wrapper for logic to detect if "Press A to Continue" should be displayed

	void UpdateProfileUserSettings( void );
	
	static void CacheAudioLevels( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _FVidDrawOverlay( void );
	void _ViewFadeDraw( void );
	void _EnableText( BOOL bEnable );


	FCLASS_STACKMEM_NOALIGN( CPlayer );
};

extern CPlayer Player_aPlayer[MAX_PLAYERS];






#endif

